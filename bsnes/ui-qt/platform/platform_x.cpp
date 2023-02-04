#include <dbus/dbus.h>

static void log(DBusError &error) {
  fprintf(stderr, "DBus error: %s\n", error.name);
  fprintf(stderr, "Message: %s\n", error.message);
}

void Application::App::inhibitScreenSaver() {
  DBusError error;
  dbus_error_init(&error);

  DBusConnection *connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
  if (connection == nullptr) {
    log(error);
    return;
  }

  DBusMessage *message = dbus_message_new_method_call(
      "org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver",
      "org.freedesktop.ScreenSaver", "Inhibit");

  const char *app = "org.bsnes.bsnes-plus";
  const char *reason = "Playing a game";
  if (!dbus_message_append_args(message, DBUS_TYPE_STRING, &app,
                                DBUS_TYPE_STRING, &reason, DBUS_TYPE_INVALID)) {
    dbus_connection_unref(connection);
    dbus_message_unref(message);
    fputs("Failed to append arguments to DBus call\n", stderr);
    return;
  }

  DBusMessage *reply = dbus_connection_send_with_reply_and_block(
      connection, message, DBUS_TIMEOUT_USE_DEFAULT, &error);
  dbus_connection_unref(connection);
  dbus_message_unref(message);
  if (reply == nullptr) {
    log(error);
    return;
  }
  dbus_message_unref(reply);
}
