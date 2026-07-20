#include "vin/notification.hpp"

#include <catch2/catch_test_macros.hpp>
#include <glib.h>

using namespace vin;

constexpr auto NOTIFICATION_FORMAT_STR{ "(susssasa{sv}i)" };

TEST_CASE("invalid type", VIN_NOTIFICATION_TAG)
{
  auto* const variant{ ::g_variant_new("u", 10) };
  const auto actual{ Notification::is_valid_type(variant) };
  ::g_variant_unref(variant);
  REQUIRE_FALSE(actual);
}

TEST_CASE("create notification", VIN_NOTIFICATION_TAG)
{
  SECTION("basic fields")
  {
    ::GVariantBuilder actions_builder;
    ::g_variant_builder_init_static(&actions_builder, G_VARIANT_TYPE_STRING_ARRAY);

    ::GVariantBuilder hints_builder;
    ::g_variant_builder_init_static(&hints_builder, G_VARIANT_TYPE("a{sv}"));

    auto* const variant{ ::g_variant_new(
      NOTIFICATION_FORMAT_STR, "app_name", 10, "app_icon", "summary", "body", &actions_builder, &hints_builder, 60) };

    const auto notification{ Notification::create() };
    notification->from_variant(variant);

    ::g_variant_unref(variant);

    CHECK(std::string_view(notification->get_app_name()) == "app_name");
    CHECK(notification->get_replaces_id() == 10);
    CHECK(std::string_view(notification->get_summary()) == "summary");
    CHECK(std::string_view(notification->get_app_icon()) == "app_icon");
    CHECK(std::string_view(notification->get_body()) == "body");
    CHECK(notification->get_expire_timeout() == 60);
  }

  SECTION("actions")
  {
    ::GVariantBuilder actions_builder;
    ::g_variant_builder_init_static(&actions_builder, G_VARIANT_TYPE_STRING_ARRAY);

    ::g_variant_builder_add(&actions_builder, "s", "key");
    ::g_variant_builder_add(&actions_builder, "s", "value");

    ::GVariantBuilder hints_builder;
    ::g_variant_builder_init_static(&hints_builder, G_VARIANT_TYPE("a{sv}"));

    auto* const variant{ ::g_variant_new(
      NOTIFICATION_FORMAT_STR, "app_name", 10, "app_icon", "summary", "body", &actions_builder, &hints_builder, 60) };

    const auto notification{ Notification::create() };
    notification->from_variant(variant);

    ::g_variant_unref(variant);

    CHECK(std::string_view(notification->get_app_name()) == "app_name");
    CHECK(notification->get_replaces_id() == 10);
    CHECK(std::string_view(notification->get_app_icon()) == "app_icon");
    CHECK(std::string_view(notification->get_summary()) == "summary");
    CHECK(std::string_view(notification->get_body()) == "body");
    CHECK(notification->get_expire_timeout() == 60);
    CHECK(notification->get_action("key") == "value");
    CHECK(notification->get_action("invalid").empty());
  }

  SECTION("hints")
  {
    ::GVariantBuilder actions_builder;
    ::g_variant_builder_init_static(&actions_builder, G_VARIANT_TYPE_STRING_ARRAY);

    ::GVariantBuilder hints_builder;
    ::g_variant_builder_init_static(&hints_builder, G_VARIANT_TYPE("a{sv}"));

    ::g_variant_builder_add(
      &hints_builder, "{sv}", "urgency", g_variant_new_byte(static_cast<int>(Notification::Urgency::normal)));
    ::g_variant_builder_add(&hints_builder, "{sv}", "category", g_variant_new_string("device.added"));
    ::g_variant_builder_add(&hints_builder, "{sv}", "desktop-entry", g_variant_new_string("foo.desktop"));

    auto* const variant{ ::g_variant_new(
      NOTIFICATION_FORMAT_STR, "app_name", 10, "app_icon", "summary", "body", &actions_builder, &hints_builder, 60) };

    const auto notification{ Notification::create() };
    notification->from_variant(variant);

    ::g_variant_unref(variant);

    CHECK(std::string_view(notification->get_app_name()) == "app_name");
    CHECK(notification->get_replaces_id() == 10);
    CHECK(std::string_view(notification->get_app_icon()) == "app_icon");
    CHECK(std::string_view(notification->get_summary()) == "summary");
    CHECK(std::string_view(notification->get_body()) == "body");
    CHECK(notification->get_expire_timeout() == 60);
    CHECK(notification->get_urgency_hint() == Notification::Urgency::normal);
    CHECK(std::string_view(notification->get_category_hint()) == "device.added");
    CHECK(std::string_view(notification->get_desktop_entry_hint()) == "foo.desktop");
  }
}
