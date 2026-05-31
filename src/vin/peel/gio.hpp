#pragma once

#include <peel/GObject/Object.h>
#include <peel/Gio/Cancellable.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/Error.h>
#include <peel/lang.h>
#include <peel/UniquePtr.h>

#include <utility>

namespace vin::gio {

template<typename Class, typename... Args>
peel::enable_if_derived<peel::Object, Class, typename peel::Object::Traits<Class>::CreateType> initable_create(
  peel::Gio::Cancellable* const cancellable,
  peel::UniquePtr<peel::GLib::Error>* const error,
  Args&&... args)
{
  ::GError* error_location; // NOLINT(cppcoreguidelines-init-variables)

  auto* const result{ peel::internals::ObjectCreateHelper<Args...>::
      template invoke<gpointer, decltype(::g_initable_new), ::GType, ::GCancellable*, ::GError**>(::g_initable_new,
        static_cast<::GType>(peel::Type::of<Class>()),
        reinterpret_cast<::GCancellable*>(cancellable),
        error ? &error_location : nullptr,
        std::forward<Args>(args)...) };

  if (error) {
    *error = peel::UniquePtr<peel::GLib::Error>::adopt_ref(reinterpret_cast<peel::GLib::Error*>(error_location));
  }

  return peel::Object::Traits<Class>::created(reinterpret_cast<::GObject*>(result));
}

} // namespace vin::gio
