/* -*- mode: C; c-file-style: "linux" -*- */
/* "corner-bar" panel applet */

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <config.h>
#include <string.h>

#include <glib/gi18n.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <gdk/gdkx.h>
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

#include <mate-panel-applet.h>
#include "common.h"

typedef struct {
    /* widgets */
    GtkWidget* applet;
    GtkWidget* box;
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
} CornerBarData;

static void toggle_showing_desktop (CornerBarData *cbd);

static gboolean
peek_callback (CornerBarData *cbd)
{
    cbd->hover_peek_timer_id = 0;

    cbd->peeking_desktop = TRUE;
    toggle_showing_desktop (cbd);

    return G_SOURCE_REMOVE;
}

static gboolean
enter_notify_callback (GtkWidget        *widget,
                       GdkEventCrossing *event,
                       gpointer          user_data)
{
    CornerBarData *cbd = (CornerBarData *) user_data;

    g_clear_handle_id (&cbd->hover_peek_timer_id, g_source_remove);

    if (!cbd->showing_desktop)
    {
        cbd->hover_peek_timer_id = g_timeout_add (cbd->hover_peek_delay,
                                                  (GSourceFunc) peek_callback,
                                                  cbd);
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
leave_event_occurred_inbounds (CornerBarData    *cbd,
                               GdkEventCrossing *event)
{
    GdkWindow *applet_window = gtk_widget_get_window (GTK_WIDGET (cbd->applet));

    gint x, y, width, height, ev_x, ev_y;

    ev_x = (gint) event->x;
    ev_y = (gint) event->y;
    gdk_window_get_geometry (applet_window, &x, &y, &width, &height);

    return ev_x >= x && ev_x < x + width &&
           ev_y >= y && ev_y < y + height;
}

static gboolean
leave_notify_callback (GtkWidget        *widget,
                       GdkEventCrossing *event,
                       gpointer          user_data)
{
    CornerBarData *cbd = (CornerBarData *) user_data;

    g_clear_handle_id (&cbd->hover_peek_timer_id, g_source_remove);

    if (leave_event_occurred_inbounds (cbd, event))
    {
        return GDK_EVENT_PROPAGATE;
    }

    if (cbd->peeking_desktop)
    {
        toggle_showing_desktop (cbd);
        cbd->peeking_desktop = FALSE;
    }

    return GDK_EVENT_PROPAGATE;
}

static void
toggle_showing_desktop (CornerBarData *cbd)
{
    if (cbd->wnck_screen != NULL)
        wnck_screen_toggle_showing_desktop (cbd->wnck_screen,
                                            !wnck_screen_get_showing_desktop (cbd->wnck_screen));
}

static void
showing_desktop_changed (WnckScreen *screen,
                         gpointer    user_data)
{
    CornerBarData *cbd = (CornerBarData *) user_data;
    cbd->showing_desktop = wnck_screen_get_showing_desktop (cbd->wnck_screen);
}

static void
button_clicked_callback (GtkWidget           *button,
                         CornerBarData       *cbd)
{
    g_clear_handle_id (&cbd->hover_peek_timer_id, g_source_remove);

    if (cbd->peeking_desktop && cbd->showing_desktop)
    {
        cbd->peeking_desktop = FALSE;
    }
    else
    {
        toggle_showing_desktop (cbd);
    }
}

/* this is when the panel orientation changes */
static void
applet_change_orient (MatePanelApplet       *applet,
                      MatePanelAppletOrient  orient,
                      CornerBarData         *cbd)
{
    GtkOrientation new_orient;

    switch (orient)
    {
        case MATE_PANEL_APPLET_ORIENT_LEFT:
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
            new_orient = GTK_ORIENTATION_VERTICAL;
            gtk_orientable_set_orientation (GTK_ORIENTABLE (cbd->box), new_orient);
            gtk_widget_set_size_request (GTK_WIDGET (applet), mate_panel_applet_get_size (applet), 12);
            break;
        case MATE_PANEL_APPLET_ORIENT_UP:
        case MATE_PANEL_APPLET_ORIENT_DOWN:
        default:
            new_orient = GTK_ORIENTATION_HORIZONTAL;
            gtk_orientable_set_orientation (GTK_ORIENTABLE (cbd->box), new_orient);
            gtk_widget_set_size_request (GTK_WIDGET (applet), 12, mate_panel_applet_get_size (applet));
            break;
    }

    gtk_widget_queue_allocate (cbd->applet);
}

static void
corner_bar_applet_realized (MatePanelApplet *applet,
                            gpointer         data)
{
    CornerBarData *cbd = (CornerBarData *) data;

    cbd->wnck_screen = wnck_screen_get_default ();
    g_signal_connect (G_OBJECT (cbd->wnck_screen),
                      "showing-desktop-changed",
                      G_CALLBACK (showing_desktop_changed), cbd);
}

static void
display_about_dialog (GtkAction     *action,
                      CornerBarData *cbd)
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
settings_changed (CornerBarData *cbd)
{
    cbd->hover_peek_enabled = g_settings_get_boolean (cbd->settings, HOVER_PEEK_KEY);
    cbd->hover_peek_delay = g_settings_get_int (cbd->settings, HOVER_PEEK_DELAY_KEY);

    if (cbd->hover_peek_enabled)
    {
        g_signal_connect (cbd->button, "enter-notify-event",
                          G_CALLBACK (enter_notify_callback),
                          cbd);
        g_signal_connect (cbd->button, "leave-notify-event",
                          G_CALLBACK (leave_notify_callback),
                          cbd);
    }
    else
    {
        g_signal_handlers_disconnect_by_func (cbd->button, enter_notify_callback, cbd);
        g_signal_handlers_disconnect_by_func (cbd->button, leave_notify_callback, cbd);
    }
}

static void applet_destroyed (GtkWidget *applet, CornerBarData *cbd)
{
    g_signal_handlers_disconnect_by_func (cbd->gtksettings, update_theme, cbd);

    g_clear_object (&cbd->settings);
    g_clear_object (&cbd->fallback_css);

    g_free (cbd);
}

static void
launch_preferences (GtkAction     *action,
                    CornerBarData *cbd)
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

static const GtkActionEntry corner_bar_menu_actions[] = {
    {
        "ShowCornerBarAbout",
        "help-about",
        N_("About"),
        NULL,
        NULL,
        G_CALLBACK (display_about_dialog)
    },
    {
        "ShowCornerBarPrefs",
        "document-properties-symbolic",
        N_("Preferences"),
        NULL,
        NULL,
        G_CALLBACK (launch_preferences)
    }
};

gboolean make_cornerbar (MatePanelApplet *applet)
{
    CornerBarData *cbd;
    GtkStyleContext *context;
    GtkActionGroup *action_group;
    gboolean can_show_desktop;

    if (GDK_IS_X11_DISPLAY (gdk_display_get_default ()))
    {
        can_show_desktop = gdk_x11_screen_supports_net_wm_hint (gdk_screen_get_default (),
                                                                gdk_atom_intern ("_NET_SHOWING_DESKTOP",
                                                                FALSE));
    }
    else
    {
        can_show_desktop = FALSE;
    }

    if (!can_show_desktop)
    {
        g_warning ("Your window manager does not support the showing the desktop, or you are not running a window manager.");
        exit(1);
    }

    mate_panel_applet_set_flags (applet, MATE_PANEL_APPLET_EXPAND_MINOR);

    cbd = g_new0 (CornerBarData, 1);
    cbd->applet = GTK_WIDGET (applet);
    cbd->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    cbd->button = gtk_button_new ();
    cbd->gtksettings = gtk_settings_get_default ();
    cbd->settings = g_settings_new (SETTINGS_SCHEMA_ID);

    g_signal_connect_swapped (cbd->gtksettings,
                              "notify::gtk-theme-name",
                              G_CALLBACK (update_theme),
                              cbd->button);
    update_theme (cbd->button);

    g_signal_connect_swapped (cbd->settings,
                              "changed",
                              G_CALLBACK (settings_changed),
                              cbd);
    settings_changed (cbd);

    context = gtk_widget_get_style_context (cbd->button);
    gtk_style_context_add_class (context, "mate-xapp-corner-bar");
    gtk_style_context_remove_class (context, "button");

    action_group = gtk_action_group_new ("Mate XApp Corner Bar Applet Actions");
    gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (action_group, corner_bar_menu_actions, G_N_ELEMENTS (corner_bar_menu_actions), cbd);
    mate_panel_applet_setup_menu_from_resource (MATE_PANEL_APPLET (cbd->applet),
                                                RESOURCE_PREFIX "mate-xapp-corner-bar-menu.xml",
                                                action_group);
    g_object_unref (action_group);

    gtk_container_add (GTK_CONTAINER (cbd->applet), cbd->box);
    gtk_box_pack_start (GTK_BOX (cbd->box), cbd->button, FALSE, FALSE, 0);
    gtk_widget_set_size_request (cbd->button, 8, -1);

    g_signal_connect (cbd->applet, "realize",
                      G_CALLBACK (corner_bar_applet_realized),
                      cbd);
    g_signal_connect (cbd->button, "clicked",
                      G_CALLBACK (button_clicked_callback),
                      cbd);
    g_signal_connect (cbd->applet, "change-orient",
                      G_CALLBACK (applet_change_orient),
                      cbd);
    g_signal_connect (cbd->applet, "destroy",
                      G_CALLBACK (applet_destroyed),
                      cbd);

    applet_change_orient (MATE_PANEL_APPLET (cbd->applet), mate_panel_applet_get_orient (MATE_PANEL_APPLET (cbd->applet)), cbd);

    gtk_widget_show_all (cbd->applet);

    return TRUE;
}

static gboolean
corner_bar_applet_factory (MatePanelApplet *applet,
                           const gchar     *iid,
                           gpointer         data)
{
    gboolean retval = FALSE;

    if (!strcmp (iid, "MateXAppCornerBarApplet"))
        retval = make_cornerbar (applet);

    if (!retval) {
        exit (-1);
    }

    return retval;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY ("MateXAppCornerBarAppletFactory",
                                       PANEL_TYPE_APPLET,
                                       "corner-bar",
                                       corner_bar_applet_factory,
                                       NULL)
