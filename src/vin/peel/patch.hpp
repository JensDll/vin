#pragma once

#include <peel/callback.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/IOChannel.h>
#include <peel/GLib/IOCondition.h>
#include <peel/GLib/Source.h>
#include <peel/RefPtr.h>

#include <type_traits>
#include <utility>

namespace vin {

void source_set_io_callback(peel::GLib::Source* const source, auto&& callback)
{
  ::gpointer out_data; // NOLINT(cppcoreguidelines-init-variables)
  ::GDestroyNotify out_notify; // NOLINT(cppcoreguidelines-init-variables)

  const ::GIOFunc func{
    peel::internals::CallbackHelper<::gboolean, ::GIOChannel*, ::GIOCondition>::wrap_notified_callback(
      std::forward<decltype(callback)>(callback),
      [](::GIOChannel* const channel, const ::GIOCondition condition, const ::gpointer data) -> ::gboolean {
        const decltype(callback)& wrapped{ *static_cast<std::remove_reference_t<decltype(callback)>*>(data) };
        return static_cast<gboolean>(
          wrapped(reinterpret_cast<peel::GLib::IOChannel*>(channel), static_cast<peel::GLib::IOCondition>(condition)));
      },
      &out_data,
      &out_notify,
      true)
  };

  ::g_source_set_callback(
    reinterpret_cast<::GSource*>(source), reinterpret_cast<::GSourceFunc>(func), out_data, out_notify);
}

template<typename T, typename... Args>
peel::RefPtr<T> initable_create(peel::Gio::Cancellable* const cancellable,
  peel::UniquePtr<peel::GLib::Error>* const error,
  Args&&... args)
{
  return peel::Gio::Initable::create(
    peel::Type::of<T>(), cancellable, reinterpret_cast<peel::GLib::Error*>(error), std::forward<Args>(args)...)
    .template cast<T>();
}

} // namespace vin
