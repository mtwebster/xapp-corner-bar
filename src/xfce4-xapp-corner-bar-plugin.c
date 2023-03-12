/*
 * Copyright (c) 2005-2007 Jasper Huijsmans <jasper@xfce.org>
 * Copyright (c) 2007-2010 Nick Schermer <nick@xfce.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

#include <libxfce4panel/xfce-panel-plugin.h>
#include "xfce4-xapp-corner-bar-plugin.h"

#include "common.h"

static void toggle_showing_desktop (XAppCornerBarPlugin *plugin);

struct _XAppCornerBarPluginClass
{
  XfcePanelPluginClass __parent__;
};

struct _XAppCornerBarPlugin
{
    XfcePanelPlugin __parent__;

    GtkWidget *box;
    GtkWidget *button;

    WnckScreen* wnck_screen;

    GSettings *settings;
    GtkSettings *gtksettings;
    GtkCssProvider *fallback_css;
    gboolean using_fallback_css;

    guint hover_peek_timer_id;
    gint hover_peek_delay;
    gboolean hover_peek_enabled;

    gboolean peeking_desktop; // active state
    gboolean showing_desktop; // reactive to wnck_screen
};

/* define the plugin */
XFCE_PANEL_DEFINE_PLUGIN (XAppCornerBarPlugin, xapp_corner_bar_plugin)

static void
xapp_corner_bar_plugin_init (XAppCornerBarPlugin *plugin)
{
}

static gboolean
peek_callback (XAppCornerBarPlugin *plugin)
{
    plugin->hover_peek_timer_id = 0;

    plugin->peeking_desktop = TRUE;
    toggle_showing_desktop (plugin);

    return G_SOURCE_REMOVE;
}

static gboolean
enter_notify_callback (GtkWidget        *widget,
                       GdkEventCrossing *event,
                       gpointer          user_data)
{
    XAppCornerBarPlugin *plugin = XAPP_CORNER_BAR_PLUGIN (user_data);

    g_clear_handle_id (&plugin->hover_peek_timer_id, g_source_remove);

    if (!plugin->showing_desktop)
    {
        plugin->hover_peek_timer_id = g_timeout_add (plugin->hover_peek_delay,
                                                     (GSourceFunc) peek_callback,
                                                     plugin);
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
leave_event_occurred_inbounds (XAppCornerBarPlugin *plugin,
                               GdkEventCrossing    *event)
{
    GdkWindow *plugin_window = gtk_widget_get_window (GTK_WIDGET (plugin));

    gint x, y, width, height, ev_x, ev_y;

    ev_x = (gint) event->x;
    ev_y = (gint) event->y;
    gdk_window_get_geometry (plugin_window, &x, &y, &width, &height);

    return ev_x >= x && ev_x <= x + width &&
           ev_y >= y && ev_y <= y + height;
}

static gboolean
leave_notify_callback (GtkWidget        *widget,
                       GdkEventCrossing *event,
                       gpointer          user_data)
{
    XAppCornerBarPlugin *plugin = XAPP_CORNER_BAR_PLUGIN (user_data);

    g_clear_handle_id (&plugin->hover_peek_timer_id, g_source_remove);

    if (leave_event_occurred_inbounds (plugin, event))
    {
        return GDK_EVENT_PROPAGATE;
    }

    if (plugin->peeking_desktop)
    {
        toggle_showing_desktop (plugin);
        plugin->peeking_desktop = FALSE;
    }

    return GDK_EVENT_PROPAGATE;
}

static void
button_clicked_callback (GtkWidget           *button,
                         XAppCornerBarPlugin *plugin)
{
    g_clear_handle_id (&plugin->hover_peek_timer_id, g_source_remove);

    if (plugin->peeking_desktop && plugin->showing_desktop)
    {
        plugin->peeking_desktop = FALSE;
    }
    else
    {
        toggle_showing_desktop (plugin);
    }
}

static gboolean
xapp_corner_bar_plugin_size_changed (XfcePanelPlugin *panel_plugin,
                                     gint             size)
{
    XAppCornerBarPlugin *plugin = XAPP_CORNER_BAR_PLUGIN (panel_plugin);

    if (xfce_panel_plugin_get_orientation (panel_plugin) == GTK_ORIENTATION_HORIZONTAL)
    {
        gtk_orientable_set_orientation (GTK_ORIENTABLE (plugin->box), GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_set_size_request (GTK_WIDGET (panel_plugin),
                                     12, size);
    }
    else
    {
        gtk_orientable_set_orientation (GTK_ORIENTABLE (plugin->box), GTK_ORIENTATION_VERTICAL);
        gtk_widget_set_size_request (GTK_WIDGET (panel_plugin),
                                     size, 12);
    }

    return TRUE;
}

static void
settings_changed (XAppCornerBarPlugin *plugin)
{
    plugin->hover_peek_enabled = g_settings_get_boolean (plugin->settings, HOVER_PEEK_KEY);
    plugin->hover_peek_delay = g_settings_get_int (plugin->settings, HOVER_PEEK_DELAY_KEY);

    if (plugin->hover_peek_enabled)
    {
        g_signal_connect (plugin->button, "enter-notify-event",
                          G_CALLBACK (enter_notify_callback),
                          plugin);
        g_signal_connect (plugin->button, "leave-notify-event",
                          G_CALLBACK (leave_notify_callback),
                          plugin);
    }
    else
    {
        g_signal_handlers_disconnect_by_func (plugin->button, enter_notify_callback, plugin);
        g_signal_handlers_disconnect_by_func (plugin->button, leave_notify_callback, plugin);
    }
}

static void
toggle_showing_desktop (XAppCornerBarPlugin *plugin)
{
    if (plugin->wnck_screen != NULL)
        wnck_screen_toggle_showing_desktop (plugin->wnck_screen,
                                            !wnck_screen_get_showing_desktop (plugin->wnck_screen));
}

static void
showing_desktop_changed (WnckScreen *screen,
                         gpointer    user_data)
{
    XAppCornerBarPlugin *plugin = XAPP_CORNER_BAR_PLUGIN (user_data);
    plugin->showing_desktop = wnck_screen_get_showing_desktop (plugin->wnck_screen);
}

static void
plugin_screen_changed (GtkWidget *widget,
                       gpointer   user_data)
{
    XAppCornerBarPlugin *plugin = XAPP_CORNER_BAR_PLUGIN (widget);
    WnckScreen *wnck_screen;
    GdkScreen  *screen;

    g_return_if_fail (XAPP_IS_CORNER_BAR_PLUGIN (widget));

    screen = gtk_widget_get_screen (widget);
    wnck_screen = wnck_screen_get (gdk_screen_get_number (screen));

    g_return_if_fail (WNCK_IS_SCREEN (wnck_screen));

    /* leave when the wnck screen did not change */
    if (plugin->wnck_screen == wnck_screen)
        return;

    /* disconnect signals from an existing wnck screen */
    if (plugin->wnck_screen != NULL)
    {
        g_signal_handlers_disconnect_by_func (G_OBJECT (plugin->wnck_screen),
                                              showing_desktop_changed,
                                              plugin);
    }

    /* set the new wnck screen */
    plugin->wnck_screen = wnck_screen;
    g_signal_connect (G_OBJECT (wnck_screen),
                      "showing-desktop-changed",
                      G_CALLBACK (showing_desktop_changed), plugin);
}

static void
show_about_dialog (GtkMenuItem *item,
                   gpointer     user_data)
{
    GtkAboutDialog *dialog;

    dialog = GTK_ABOUT_DIALOG (gtk_about_dialog_new ());

    gtk_about_dialog_set_program_name (dialog, _("XApp CornerBar Plugin"));
    gtk_about_dialog_set_version (dialog, VERSION);
    gtk_about_dialog_set_license_type (dialog, GTK_LICENSE_GPL_3_0);
    gtk_about_dialog_set_website (dialog, "https://www.github.com/linuxmint/xapp-corner-bar");
    gtk_about_dialog_set_logo_icon_name (dialog, "xapp-corner-bar");
    gtk_about_dialog_set_comments (dialog, _("This widget shows the desktop when clicked, or can peek at the desktop when hovered."));

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
add_about_menu_item (XAppCornerBarPlugin *plugin)
{
    GtkMenuItem *item;

    item = GTK_MENU_ITEM (gtk_menu_item_new_with_label (_("About")));
    gtk_widget_show (GTK_WIDGET (item));

    g_signal_connect (item,
                      "activate",
                      G_CALLBACK (show_about_dialog),
                      plugin);

    xfce_panel_plugin_menu_insert_item (XFCE_PANEL_PLUGIN (plugin),
                                        item);
}

static void
xapp_corner_bar_plugin_construct (XfcePanelPlugin *panel_plugin)
{
    XAppCornerBarPlugin *plugin = XAPP_CORNER_BAR_PLUGIN (panel_plugin);
    GtkStyleContext *context;

    g_signal_connect (panel_plugin,
                      "screen-changed",
                      G_CALLBACK (plugin_screen_changed),
                      NULL);

    if (gtk_widget_get_realized (GTK_WIDGET (panel_plugin)))
    {
        plugin_screen_changed (GTK_WIDGET (panel_plugin), NULL);
    }

    plugin->box = gtk_box_new (xfce_panel_plugin_get_orientation (panel_plugin), 0);
    plugin->button = gtk_button_new ();

    plugin->gtksettings = gtk_settings_get_default ();
    plugin->settings = g_settings_new (SETTINGS_SCHEMA_ID);

    g_signal_connect_swapped (plugin->gtksettings,
                              "notify::gtk-theme-name",
                              G_CALLBACK (update_theme),
                              plugin->button);
    update_theme (plugin->button);

    g_signal_connect_swapped (plugin->settings,
                              "changed",
                              G_CALLBACK (settings_changed),
                              plugin);
    settings_changed (plugin);

    context = gtk_widget_get_style_context (plugin->button);
    gtk_style_context_add_class (context, "mate-xapp-corner-bar");
    gtk_style_context_remove_class (context, "button");

    gtk_container_add (GTK_CONTAINER (plugin), plugin->box);
    gtk_box_pack_start (GTK_BOX (plugin->box), plugin->button, FALSE, FALSE, 0);

    g_signal_connect (plugin->button, "clicked",
                      G_CALLBACK (button_clicked_callback),
                      plugin);
    g_signal_connect (panel_plugin, "size-changed",
                      G_CALLBACK (gtk_true),
                      NULL);

    gtk_widget_show_all (GTK_WIDGET (plugin));

    xfce_panel_plugin_set_small (panel_plugin, TRUE);

    if (g_find_program_in_path ("xapp-corner-bar-prefs"))
    {
        xfce_panel_plugin_menu_show_configure (panel_plugin);
    }

    add_about_menu_item (plugin);
}

/* This is our dispose */
static void
xapp_corner_bar_plugin_free_data (XfcePanelPlugin *panel_plugin)
{
    XAppCornerBarPlugin *plugin = XAPP_CORNER_BAR_PLUGIN (panel_plugin);

    if (plugin->wnck_screen != NULL)
    {
        g_signal_handlers_disconnect_by_func (G_OBJECT (plugin->wnck_screen),
                                              showing_desktop_changed,
                                              plugin);
    }

    g_signal_handlers_disconnect_by_func (plugin->gtksettings, update_theme, plugin);

    g_clear_object (&plugin->settings);
    g_clear_object (&plugin->fallback_css);
}

static void
xapp_corner_bar_plugin_configure_plugin (XfcePanelPlugin *panel_plugin)
{
    GError *error = NULL;

    const gchar *argv[2] = {
        PREFS_PATH,
        NULL
    };

    if (!g_spawn_async (NULL,
                        (gchar **) argv,
                        NULL,
                        G_SPAWN_DEFAULT,
                        NULL,
                        NULL,
                        NULL,
                        &error))
    {
        g_critical ("Could not launch xapp-corner-bar-prefs (%d): %s", error->code, error->message);
        g_error_free (error);
    }
}

static void
xapp_corner_bar_plugin_class_init (XAppCornerBarPluginClass *klass)
{
  XfcePanelPluginClass *plugin_class;

  plugin_class = XFCE_PANEL_PLUGIN_CLASS (klass);
  plugin_class->construct = xapp_corner_bar_plugin_construct;
  plugin_class->free_data = xapp_corner_bar_plugin_free_data;
  plugin_class->size_changed = xapp_corner_bar_plugin_size_changed;
  plugin_class->configure_plugin = xapp_corner_bar_plugin_configure_plugin;

    /* Initialize gettext support */
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
}
