#import "common.h"

static GtkCssProvider *fallback_css = NULL;
static gboolean using_fallback_css = FALSE;

void
update_theme (GtkWidget *widget)
{
    GtkStyleContext *context;
    GtkCssProvider *current;
    gchar *theme_name, *css;

    g_object_get (gtk_settings_get_default (), "gtk-theme-name", &theme_name, NULL);
    current = gtk_css_provider_get_named (theme_name, NULL);
    css = gtk_css_provider_to_string (current);

    if (!g_strstr_len (css, -1, "xapp-corner-bar"))
    {
        if (fallback_css == NULL)
        {
            fallback_css = gtk_css_provider_new ();
            gtk_css_provider_load_from_resource (fallback_css, RESOURCE_PREFIX "xapp-corner-bar.css");
        }

        if (!using_fallback_css)
        {
            context = gtk_widget_get_style_context (widget);
            gtk_style_context_add_provider (context,
                                            GTK_STYLE_PROVIDER (fallback_css),
                                            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            using_fallback_css = TRUE;
        }
    }
    else
    {
        if (using_fallback_css)
        {
            context = gtk_widget_get_style_context (widget);
            gtk_style_context_remove_provider (context, GTK_STYLE_PROVIDER (fallback_css));
            using_fallback_css = FALSE;
        }
    }

    g_free (theme_name);
    g_free (css);
}
