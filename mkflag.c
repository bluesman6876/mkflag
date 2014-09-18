#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <pango/pangocairo.h>
#include "config.h"

#define DRAWING_AREA_WIDTH	2048
#define DRAWING_AREA_HEIGHT	2048

enum {
	TEXT_COLOR, FLAG_COLOR, BORDER_COLOR, COLOR_NUMBER
};

static struct flag {
	double scale;
	double height;
	double width;
	struct text {
		double width;
		double x, y;
		const char *str;
#if defined(PANGO)
		struct font {
			PangoLayout *layout;
			PangoFontDescription *desc;
		} font;
#endif				/* #if defined(PANGO) */
	} text;
	const char *fnp;
	struct logo {
		const char *fn;
		double width, height;
		GdkPixbuf *img;
	} logo;
	struct stock {
		enum style {
			STYLE_NONE, STYLE_LEFT_TOP, STYLE_LEFT_CENTER,
			STYLE_LEFT_BOTTOM, STYLE_TOP_LEFT, STYLE_TOP_CENTER,
			STYLE_TOP_RIGHT, STYLE_RIGHT_TOP, STYLE_RIGHT_CENTER,
			STYLE_RIGHT_BOTTOM, STYLE_BOTTOM_RIGHT,
			STYLE_BOTTOM_CENTER, STYLE_BOTTOM_LEFT,	STYLE_NUM
		} style;
		double height;
		double width;
	} stock;
	struct rgba_color {
		double a;
		double r;
		double g;
		double b;
	} color[COLOR_NUMBER];
} flag;

static const char *style_name[STYLE_NUM] = {
	"none", "lt", "lc", "lb", "tl", "tc", "tr", "rt", "rc", "rb", "br",
	"bc", "bl"
};

#define FONT_SIZE		16.0
#define MARGIN_LR		2.0
#define BORDER_WIDTH		2.0
#define RADIUS			8.0
#define MARGIN_LOGO		8.0

static int flag_init(cairo_t *cr, struct flag *f)
{
	int retval = -1;
#if defined(PANGO)
	struct font *font;

	font = &f->text.font;
	memset(font, 0, sizeof(*font));
	font->desc = pango_font_description_new();

	if (!font->desc) {
		g_error("pango_font_description_new");
		goto exit;
	}

	pango_font_description_set_family(font->desc, "arial");
	pango_font_description_set_weight(font->desc, PANGO_WEIGHT_BOLD);
	pango_font_description_set_absolute_size(font->desc,
	    FONT_SIZE * PANGO_SCALE);
	font->layout = pango_cairo_create_layout(cr);

	if (!font->layout) {
		g_error("pango_cairo_create_layout");
		goto exit;
	}

	pango_layout_set_font_description(font->layout, font->desc);
	f->text.width = 100;
	f->width = f->text.width + 2 * (MARGIN_LR + BORDER_WIDTH);
	f->text.x = MARGIN_LR + BORDER_WIDTH;
	f->text.y = 64;
	f->height = 2 * 64;
	g_debug("height %f", f->height);
#else				/* #if defined(PANGO) */
	cairo_text_extents_t te;
	cairo_font_extents_t fe;

	cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL,
	    CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, FONT_SIZE);
	cairo_font_extents (cr, &fe);
	cairo_text_extents(cr, f->text.str, &te);
	g_debug("font: ascent %f descent %f height %f max_x_advance %f "
	    "max_y_advance %f", fe.ascent, fe.descent, fe.height,
	    fe.max_x_advance, fe.max_y_advance);
	g_debug("text: x_bearing %f y_bearing %f width %f height %f "
	    "x_advance %f y_advance %f", te.x_bearing, te.y_bearing, te.width,
	    te.height, te.x_advance, te.y_advance);
	f->text.width = te.width;
	f->width = f->text.width + 2 * (MARGIN_LR + BORDER_WIDTH);
	f->text.x = MARGIN_LR + BORDER_WIDTH;
	f->text.y = fe.height;
	f->height = 2 * fe.height;
	f->stock.height = f->height / 2;
	f->stock.width = f->stock.height / 2;
	cairo_text_extents(cr, "о", &te);
	g_debug("text: x_bearing %f y_bearing %f width %f height %f "
	    "x_advance %f y_advance %f", te.x_bearing, te.y_bearing, te.width,
	    te.height, te.x_advance, te.y_advance);
	f->height -= te.height;
	g_debug("height %f text %f %f", f->height, f->text.x, f->text.y);

	if (!f->logo.fn) {
		goto exit;
	}

	f->logo.height = round(f->height - MARGIN_LOGO);
	f->logo.img = gdk_pixbuf_new_from_file_at_scale(f->logo.fn, -1,
	    f->logo.height, TRUE, NULL);

	if (!f->logo.img) {
		g_warning("gdk_pixbuf_new_from_file_at_scale");
		goto exit;
	}

	f->logo.width = gdk_pixbuf_get_width(f->logo.img);
	f->width += f->logo.width + MARGIN_LOGO / 2;
	f->text.x += f->logo.width + MARGIN_LOGO / 2;
	f->text.width += f->logo.width + MARGIN_LOGO / 2;
	g_debug("logo width %f height %f", f->logo.width, f->logo.height);
exit:
#endif				/* #else */
	retval = 0;
#if defined(PANGO)
exit:
#else				/* #if defined(PANGO) */
#endif				/* #else */
	return retval;
}

static void flag_setup(struct flag *f, enum style style, double scale,
    const char *text, const unsigned int *color, const char *fnp,
    const char *logo)
{
	int i;

	memset(f, 0, sizeof(*f));
	f->scale = scale;
	f->stock.style = style;
	f->text.str = text;
	f->fnp = fnp;
	f->logo.fn = logo;

	if (color) {
		for (i = 0; i < COLOR_NUMBER; i++) {
			f->color[i].a = (color[i] >> 24 & 0xff) / 255.0;
			f->color[i].r = (color[i] >> 16 & 0xff) / 255.0;
			f->color[i].g = (color[i] >>  8 & 0xff) / 255.0;
			f->color[i].b = (color[i] >>  0 & 0xff) / 255.0;
		}
	}
}

static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  	struct flag *f;
	const double degrees = M_PI / 180.0;
	cairo_surface_t *cs = NULL;
	char *fn = NULL;
	size_t fn_len;
	double x, y, width, height;
	cairo_status_t status;

	f = (struct flag *)data;
	g_assert(f);
	g_assert(f->text.str);
	g_assert(f->fnp);

	if (!f || !f->text.str || !f->fnp) {
		goto exit;
	}

	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	cairo_scale(cr, f->scale, f->scale);

	if (flag_init(cr, f) < 0) {
		g_error("flag_init");
		goto exit;
	}

	x = y = 1;
	width = f->width + 1;
	height = f->height + 1;

	switch (f->stock.style) {
	case STYLE_TOP_LEFT:
	case STYLE_TOP_RIGHT:
	case STYLE_TOP_CENTER:
		y += f->stock.height + 1;
		break;

	case STYLE_BOTTOM_LEFT:
	case STYLE_BOTTOM_RIGHT:
	case STYLE_BOTTOM_CENTER:
		height += f->stock.height + 1;
		break;

	case STYLE_LEFT_TOP:
	case STYLE_LEFT_CENTER:
	case STYLE_LEFT_BOTTOM:
		x += f->stock.height + 1;
		break;

	case STYLE_RIGHT_TOP:
	case STYLE_RIGHT_CENTER:
	case STYLE_RIGHT_BOTTOM:
		width += f->stock.height + 1;
		break;

	default:
		break;
	}

	width = ceil(width);
	height = ceil(height);
	g_debug("style %d", f->stock.style);
	g_debug("x %f y %f width %f height %f", x, y, width, height);
	g_debug("%f text %f stock %f logo %f", f->width, f->text.width, f->stock.height,
	    f->logo.width);
	cairo_translate(cr, x, y);
	cs = cairo_get_target(cr);
	status = cairo_surface_status(cs);

	if (status != CAIRO_STATUS_SUCCESS) {
		g_warning("%s", cairo_status_to_string(status));
		goto exit;
	}

	cairo_user_to_device(cr, &width, &height);
	cs = cairo_surface_create_for_rectangle(cs, 0, 0, width, height);
	status = cairo_surface_status(cs);

	if (status != CAIRO_STATUS_SUCCESS) {
		g_warning("%s", cairo_status_to_string(status));
		goto exit;
	}

	if ((f->width + 2 * (BORDER_WIDTH + f->stock.height)) * f->scale >
	    DRAWING_AREA_WIDTH ||
	    (f->height + 2 * (BORDER_WIDTH + f->stock.height)) * f->scale >
	    DRAWING_AREA_WIDTH) {
		g_warning("image clipping!!!");
		goto exit;
	}

	cairo_new_sub_path(cr);
	cairo_set_source_rgba(cr, f->color[FLAG_COLOR].r,
	    f->color[FLAG_COLOR].g, f->color[FLAG_COLOR].b,
	    f->color[FLAG_COLOR].a);
	cairo_arc(cr, RADIUS, RADIUS, RADIUS, 180 * degrees, 270 * degrees);

	if (f->stock.style == STYLE_TOP_LEFT) {
		cairo_line_to(cr, 0, -f->stock.height);
		cairo_line_to(cr, RADIUS + f->stock.width, 0);
	} else if (f->stock.style == STYLE_TOP_CENTER) {
		cairo_line_to(cr,
		    MARGIN_LR + (f->text.width - f->stock.width) / 2, 0);
		cairo_line_to(cr, MARGIN_LR + f->text.width / 2,
		    -f->stock.height);
		cairo_line_to(cr,
		    MARGIN_LR + (f->text.width + f->stock.width) / 2, 0);
	} else if (f->stock.style == STYLE_TOP_RIGHT) {
		cairo_line_to(cr, f->width - RADIUS - f->stock.width, 0);
		cairo_line_to(cr, f->width, -f->stock.height);
		cairo_line_to(cr, f->width - RADIUS, 0);
	}

	cairo_arc(cr, f->width - RADIUS, RADIUS, RADIUS, -90 * degrees,
	    0 * degrees);

	if (f->stock.style == STYLE_RIGHT_TOP) {
		cairo_line_to(cr, f->width + f->stock.height, 0);
		cairo_line_to(cr, f->width, RADIUS + f->stock.width);
	} else if (f->stock.style == STYLE_RIGHT_CENTER) {
		cairo_line_to(cr, f->width,
		    (f->height - f->stock.width) / 2);
		cairo_line_to(cr, f->width + f->stock.height,
		    f->height / 2);
		cairo_line_to(cr, f->width, (f->height + f->stock.width) / 2);
	} else if (f->stock.style == STYLE_RIGHT_BOTTOM) {
		cairo_line_to(cr, f->width,
		    f->height - f->stock.width - RADIUS);
		cairo_line_to(cr, f->width + f->stock.height, f->height);
		cairo_line_to(cr, f->width, f->height - RADIUS);
	}

	cairo_arc(cr, f->width - RADIUS, f->height - RADIUS, RADIUS,
	    0 * degrees, 90 * degrees);

	if (f->stock.style == STYLE_BOTTOM_RIGHT) {
		cairo_line_to(cr, f->width, f->height + f->stock.height);
		cairo_line_to(cr, f->width - RADIUS - f->stock.width, f->height);
	} else if (f->stock.style == STYLE_BOTTOM_CENTER) {
		cairo_line_to(cr,
		    MARGIN_LR + (f->text.width + f->stock.width) / 2,
		    f->height);
		cairo_line_to(cr, MARGIN_LR + f->text.width / 2,
		    f->height + f->stock.height);
		cairo_line_to(cr,
		    MARGIN_LR + (f->text.width - f->stock.width) / 2,
		    f->height);
	} else if (f->stock.style == STYLE_BOTTOM_LEFT) {
		cairo_line_to(cr, RADIUS + f->stock.width, f->height);
		cairo_line_to(cr, 0, f->height + f->stock.height);
		cairo_line_to(cr, RADIUS, f->height);
	}

	cairo_arc(cr, RADIUS, f->height - RADIUS, RADIUS, 90 * degrees,
	    180 * degrees);

	if (f->stock.style == STYLE_LEFT_BOTTOM) {
		cairo_line_to(cr, -f->stock.height, f->height);
		cairo_line_to(cr, 0, f->height - f->stock.width - RADIUS);
	} else if (f->stock.style == STYLE_LEFT_CENTER) {
		cairo_line_to(cr, 0, (f->height + f->stock.width) / 2);
		cairo_line_to(cr, -f->stock.height, f->height / 2);
		cairo_line_to(cr, 0, (f->height - f->stock.width) / 2);
	} else if (f->stock.style == STYLE_LEFT_TOP) {
		cairo_line_to(cr, 0, RADIUS + f->stock.width);
		cairo_line_to(cr, -f->stock.height, 0);
		cairo_line_to(cr, 0, RADIUS);
	}

	cairo_close_path(cr);
	cairo_fill_preserve(cr);
	cairo_set_source_rgba(cr, f->color[BORDER_COLOR].r,
	    f->color[BORDER_COLOR].g, f->color[BORDER_COLOR].b,
	    f->color[BORDER_COLOR].a);
	cairo_set_line_width(cr, BORDER_WIDTH);
	cairo_stroke(cr);
	cairo_set_source_rgba(cr, f->color[TEXT_COLOR].r,
	    f->color[TEXT_COLOR].g, f->color[TEXT_COLOR].b,
	    f->color[TEXT_COLOR].a);
	cairo_move_to(cr, f->text.x, f->text.y);
#if defined(PANGO)
	pango_layout_set_text(f->text.font.layout, f->text.str, -1);
	pango_cairo_show_layout_line(cr,
	    pango_layout_get_line(f->text.font.layout, 0));
#else				/* #if defined(PANGO) */
	cairo_show_text(cr, f->text.str);
#endif				/* #else */
	if (f->logo.img) {
		gdk_cairo_set_source_pixbuf(cr, f->logo.img, MARGIN_LOGO / 2,
		    MARGIN_LOGO / 2);
		cairo_rectangle(cr, MARGIN_LOGO / 2, MARGIN_LOGO / 2,
		    f->logo.width, f->logo.height);
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
		cairo_fill(cr);
	}

	fn_len = strlen(f->fnp) + strlen(style_name[f->stock.style]) + 10;
	fn = g_malloc(fn_len);

	if (snprintf(fn, fn_len, "%s_%s_%.1f.png", f->fnp,
	    style_name[f->stock.style], f->scale) > fn_len) {
		g_warning("file name %s truncated!!!", fn);
	}

	status = cairo_surface_write_to_png(cs, fn);

	if (status != CAIRO_STATUS_SUCCESS) {
		g_warning("%s", cairo_status_to_string(status));
	}
exit:
	if (fn) {
		g_free(fn);
	}

	if (cs) {
		cairo_surface_destroy(cs);
	}

	if (f->logo.img) {
		g_object_unref(f->logo.img);
	}
#if defined(PANGO)
	if (f->text.font.layout) {
		g_object_unref(f->text.font.layout);
	}

	if (f->text.font.desc) {
		pango_font_description_free(f->text.font.desc);
	}
#else				/* #if defined(PANGO) */
#endif				/* #else */
	return FALSE;
}

static void usage(void)
{
	printf("usage:\nmkfont [-t <\"text\"> -c <text color,flag color,"
	    "border color> -f <file name prefix> -s <scale> "\
	    "[-l <logo file name>]] [-h] [-V]\n"
	    "-t set text tag\n"
	    "-c set color AARRGGBB (in hex)\n"
	    "-f set file name prefix\n"
	    "-s set scale\n"
	    "-l set logo file name\n"
	    "-h print this help\n"
	    "-V print version\n"
	    );
}

int main(int argc, char *argv[])
{
	int retval, c;
	struct {
		struct {
			gint t	: 1;
			gint c	: 1;
			gint f	: 1;
			gint s	: 1;
			gint l	: 1;
			gint V	: 1;
			gint h	: 1;
		} map;
		char *fnp, *logo;
		unsigned int color[COLOR_NUMBER];
		char *text;
		double scale;
	} param;
	GtkWidget *window;
	GtkWidget *drawing_area;

	g_log_set_handler(NULL, G_LOG_LEVEL_MASK, g_log_default_handler, NULL);
	memset(&param, 0, sizeof(param));

        while ((c = getopt(argc, argv, "t:c:f:s:l:hV")) != -1) {
		switch (c) {
		case 't':
			param.map.t = 1;
			param.text = optarg;
			g_debug("text tag %s", param.text);
			break;

		case 'c':
			if (sscanf(optarg, "%08x,%08x,%08x",
			    &param.color[TEXT_COLOR], &param.color[FLAG_COLOR],
			    &param.color[BORDER_COLOR]) == 3) {
				param.map.c = 1;
				g_debug("color text %08x flag %08x border "
				    "%08x", param.color[TEXT_COLOR],
				    param.color[FLAG_COLOR],
				    param.color[BORDER_COLOR]);
			}
			break;

		case 'f':
			param.map.f = 1;
			param.fnp = optarg;
			g_debug("file name prefix %s", param.fnp);
			break;

		case 's':
			param.map.s = 1;
			sscanf(optarg, "%lf", &param.scale);
			g_debug("scale %f", param.scale);
			break;

		case 'l':
			param.map.l = 1;
			param.logo = optarg;
			g_debug("logo file name %s", param.logo);
			break;

		case 'V':
			param.map.V = 1;
			break;

		case 'h':
			param.map.h = 1;
			break;
		}
	}

	if (param.map.t && param.map.c && param.map.f && param.map.s) {
		flag_setup(&flag, 0, 0, NULL, NULL, NULL, NULL);
		gtk_init(&argc, &argv);
		window = gtk_offscreen_window_new();
		drawing_area = gtk_drawing_area_new();
		gtk_widget_set_size_request(drawing_area, DRAWING_AREA_WIDTH,
		    DRAWING_AREA_HEIGHT);
		gtk_container_add(GTK_CONTAINER(window), drawing_area);
		g_signal_connect(G_OBJECT(drawing_area), "draw",
		    G_CALLBACK(draw_callback), &flag);
		gtk_widget_show_all(window);

		for (c = STYLE_NONE; c < STYLE_NUM; c++) {
			flag_setup(&flag, c, param.scale, param.text,
			    param.color, param.fnp, param.logo);
			gtk_widget_queue_draw_area(window, 0, 0,
			    DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT);
			gdk_window_process_updates(
			    gtk_widget_get_window(window), TRUE);
		}

		retval = 0;
	} else if (param.map.V) {
		printf("mkflag version " VERSION "\n");
		retval = 0;
	} else {
		usage();
		retval = param.map.h ? 0 : -1;
	}

	return retval;
}
