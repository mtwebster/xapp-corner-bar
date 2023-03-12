/*
 * Copyright (c) 2008-2010 Nick Schermer <nick@xfce.org>
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

#ifndef __XAPP_CORNER_BAR_PLUGIN_H__
#define __XAPP_CORNER_BAR_PLUGIN_H__

#include <glib-object.h>
#include <libxfce4panel/libxfce4panel.h>

G_BEGIN_DECLS

typedef struct _XAppCornerBarPluginClass XAppCornerBarPluginClass;
typedef struct _XAppCornerBarPlugin      XAppCornerBarPlugin;

#define XAPP_TYPE_CORNER_BAR_PLUGIN            (xapp_corner_bar_plugin_get_type ())
#define XAPP_CORNER_BAR_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XAPP_TYPE_CORNER_BAR_PLUGIN, XAppCornerBarPlugin))
#define XAPP_CORNER_BAR_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XAPP_TYPE_CORNER_BAR_PLUGIN, XAppCornerBarPluginClass))
#define XAPP_IS_CORNER_BAR_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XAPP_TYPE_CORNER_BAR_PLUGIN))
#define XAPP_IS_CORNER_BAR_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XAPP_TYPE_CORNER_BAR_PLUGIN))
#define XAPP_CORNER_BAR_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XAPP_TYPE_CORNER_BAR_PLUGIN, XAppCornerBarPluginClass))

GType xapp_corner_bar_plugin_get_type      (void) G_GNUC_CONST;

void  xapp_corner_bar_plugin_register_type (XfcePanelTypeModule *type_module);

G_END_DECLS

#endif /* !__XAPP_CORNER_BAR_PLUGIN_H__ */
