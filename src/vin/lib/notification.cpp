#include "vin/lib/notification.hpp"

#include <glib.h>
#include <peel/class.h>
#include <peel/GLib/Variant.h>
#include <peel/GLib/VariantIter.h>
#include <peel/GLib/VariantType.h>
#include <peel/UniquePtr.h>
#include <spdlog/spdlog.h>

#include <string_view>
#include <vector>

using namespace peel;
using namespace vin::lib;

namespace {

gsize read_string(::GVariant* const str, std::vector<char>& buffer)
{
  gsize length; // NOLINT(cppcoreguidelines-init-variables)
  const auto* const data{ ::g_variant_get_string(str, &length) };
  buffer.append_range(std::string_view(data, length + 1));
  ::g_variant_unref(str);
  return length;
}

gsize read_string(::GVariantIter* const it, std::vector<char>& buffer)
{
  return read_string(::g_variant_iter_next_value(it), buffer);
}

class StringView
{
private:
  gsize m_size{};
  gsize m_start{};

public:
  StringView() = default;

  StringView(const std::vector<char>& buffer, const gsize size)
    : m_size{ size },
      m_start{ buffer.size() - m_size - 1 }
  {}

  [[nodiscard]] std::string_view to_string_view(const std::vector<char>& buffer) const
  {
    return { data(buffer), m_size };
  }

  [[nodiscard]] const char* data(const std::vector<char>& buffer) const
  {
    return buffer.data() + m_start;
  }

  [[nodiscard]] gsize size() const
  {
    return m_size;
  }
};

std::vector<StringView> read_string_array(::GVariant* const arr, std::vector<char>& buffer)
{
  std::vector<StringView> result;

  ::GVariantIter it;
  ::g_variant_iter_init(&it, arr);
  ::g_variant_unref(arr);

  ::GVariant* value; // NOLINT(cppcoreguidelines-init-variables)
  while ((value = ::g_variant_iter_next_value(&it)) != nullptr) {
    result.emplace_back(buffer, read_string(value, buffer));
  }

  return result;
}

struct Hints
{
  StringView category;
  StringView desktop_entry;
  Notification::Urgency urgency;
};

Hints read_hints(::GVariant* const hints, std::vector<char>& buffer)
{
  ::GVariantDict dict;
  ::g_variant_dict_init(&dict, hints);
  ::g_variant_unref(hints);

  Notification::Urgency urgency{};
  auto* value{ ::g_variant_dict_lookup_value(&dict, "urgency", G_VARIANT_TYPE_BYTE) };
  if (value != nullptr) {
    urgency = Notification::Urgency{ ::g_variant_get_byte(value) };
    ::g_variant_unref(value);
  }

  StringView category;
  value = ::g_variant_dict_lookup_value(&dict, "category", G_VARIANT_TYPE_STRING);
  if (value != nullptr) {
    category = StringView(buffer, read_string(value, buffer));
  }

  StringView desktop_entry;
  value = ::g_variant_dict_lookup_value(&dict, "desktop-entry", G_VARIANT_TYPE_STRING);
  if (value != nullptr) {
    desktop_entry = StringView(buffer, read_string(value, buffer));
  }

  ::g_variant_dict_clear(&dict);

  return {
    .category = category,
    .desktop_entry = desktop_entry,
    .urgency = urgency,
  };
}

} // namespace

bool Notification::is_valid_type(::GVariant* const variant)
{
  auto* const expected_type{ ::g_variant_type_new("(susssasa{sv}i)") };
  const bool result{ ::g_variant_is_of_type(variant, expected_type) != 0 };
  ::g_variant_type_free(expected_type);
  return result;
}

PEEL_CLASS_IMPL(Notification, "VinLibNotification", Object)

void Notification::Class::init() {}

void Notification::init([[maybe_unused]] Class* const cls)
{
  new (&m_actions) decltype(m_actions);
  new (&m_buffer) decltype(m_buffer);
}

void Notification::from_variant(::GVariant* const notification)
{
  ::GVariantIter it;
  ::g_variant_iter_init(&it, notification);

  const StringView app_name(m_buffer, read_string(&it, m_buffer));

  g_assert(::g_variant_iter_next(&it, "u", &m_replaces_id));
  m_id = m_replaces_id;

  const StringView app_icon(m_buffer, read_string(&it, m_buffer));

  const StringView summary(m_buffer, read_string(&it, m_buffer));

  const StringView body(m_buffer, read_string(&it, m_buffer));

  const auto actions{ read_string_array(::g_variant_iter_next_value(&it), m_buffer) };

  const auto hints{ read_hints(::g_variant_iter_next_value(&it), m_buffer) };

  g_assert(::g_variant_iter_next(&it, "i", &m_expire_timeout));

  g_assert(::g_variant_iter_next_value(&it) == nullptr);

  for (gsize i{}; i + 1 < actions.size(); ++i) {
    m_actions.try_emplace(actions[i].to_string_view(m_buffer), actions[i + 1].data(m_buffer), actions[i + 1].size());
  }

  m_app_name = app_name.to_string_view(m_buffer);
  m_app_icon = app_icon.to_string_view(m_buffer);
  m_summary = summary.to_string_view(m_buffer);
  m_body = body.to_string_view(m_buffer);
  m_category_hint = hints.category.to_string_view(m_buffer);
  m_desktop_entry_hint = hints.desktop_entry.to_string_view(m_buffer);
  m_urgency_hint = hints.urgency;
}
