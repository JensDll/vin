#include "vin/lib/notification.hpp"

#include <catch2/catch_test_macros.hpp>
#include <glib.h>
#include <peel/GLib/Variant.h>

using namespace peel;
using namespace vin::lib;

constexpr auto NOTIFICATION_FORMAT_STR{ "(susssasa{sv}i)" };

TEST_CASE("invalid type", VIN_TEST_TAG)
{
  auto* const variant{ ::g_variant_new("u", 10) };
  const auto actual{ Notification::is_valid_type(variant) };
  ::g_variant_unref(variant);
  REQUIRE_FALSE(actual);
}

TEST_CASE("create notification", VIN_TEST_TAG)
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

    CHECK(notification->app_name() == "app_name");
    CHECK(notification->replaces_id() == 10);
    CHECK(notification->summary() == "summary");
    CHECK(notification->app_icon() == "app_icon");
    CHECK(notification->body() == "body");
    CHECK(notification->expire_timeout() == 60);
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

    CHECK(notification->app_name() == "app_name");
    CHECK(notification->replaces_id() == 10);
    CHECK(notification->app_icon() == "app_icon");
    CHECK(notification->summary() == "summary");
    CHECK(notification->body() == "body");
    CHECK(notification->expire_timeout() == 60);
    CHECK(notification->action("key") == "value");
    CHECK(notification->action("invalid").empty());
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

    CHECK(notification->app_name() == "app_name");
    CHECK(notification->replaces_id() == 10);
    CHECK(notification->app_icon() == "app_icon");
    CHECK(notification->summary() == "summary");
    CHECK(notification->body() == "body");
    CHECK(notification->expire_timeout() == 60);
    CHECK(notification->urgency_hint() == Notification::Urgency::normal);
    CHECK(notification->category_hint() == "device.added");
    CHECK(notification->desktop_entry_hint() == "foo.desktop");
  }
}
