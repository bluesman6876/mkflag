#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
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
	double text_width;
	double text_x, text_y;
	char *text, *fnp;
	struct stock {
		enum style{
			STOCK_NONE, STOCK_LEFT_TOP, STOCK_LEFT_CENTER,
			STOCK_LEFT_BOTTOM, STOCK_TOP_LEFT, STOCK_TOP_CENTER,
			STOCK_TOP_RIGHT, STOCK_RIGHT_TOP, STOCK_RIGHT_CENTER,
			STOCK_RIGHT_BOTTOM, STOCK_BOTTOM_RIGHT,
			STOCK_BOTTOM_CENTER, STOCK_BOTTOM_LEFT,	STOCK_NUM
		} style;
	} stock;
	struct rgba_color {
		double r;
		double g;
		double b;
		double a;
	} color[COLOR_NUMBER];
} flag;

static char *stock_name[STOCK_NUM] = {
	"none", "lt", "lc", "lb", "tl", "tc", "tr", "rt", "rc", "rb", "br", "bc",
	"bl"
};

#define FONT_SIZE		16.0
#define MARGIN_LR		2.0
#define BORDER_WIDTH		2.0
#define STOCK_WIDTH		18.0
#define STOCK_HEIGHT		7.0
#define RADIUS			8.0

static void flag_init(cairo_t *cr, struct flag *f)
{
	cairo_text_extents_t te;
	cairo_font_extents_t fe;

	cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL,
	    CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, FONT_SIZE);
	cairo_font_extents (cr, &fe);
	cairo_text_extents(cr, f->text, &te);
	g_debug("font: ascent %f descent %f height %f max_x_advance %f "
			"max_y_advance %f\n", fe.ascent, fe.descent, fe.height,
			fe.max_x_advance, fe.max_y_advance);
	g_debug("text: x_bearing %f y_bearing %f width %f height %f "
			"x_advance %f y_advance %f\n", te.x_bearing, te.y_bearing, te.width,
			te.height, te.x_advance, te.y_advance);
	f->text_width = te.width;
	f->width = f->text_width + 2 * (MARGIN_LR + BORDER_WIDTH);
	f->text_x = MARGIN_LR + BORDER_WIDTH;
	f->text_y = fe.height;
	f->height = 2 * fe.height;
	cairo_text_extents(cr, "о", &te);
	g_debug("text: x_bearing %f y_bearing %f width %f height %f "
			"x_advance %f y_advance %f\n", te.x_bearing, te.y_bearing, te.width,
			te.height, te.x_advance, te.y_advance);
	f->height -= te.height;
	g_debug("height %f text %f %f\n", f->height, f->text_x, f->text_y);
}

static void flag_setup(struct flag *f, enum style style, double scale,
    char *text, unsigned int *color, char *fnp)
{
	int i;

	f->scale = scale;
	f->stock.style = style;
	f->text = text;
	f->fnp = fnp;

	for (i = 0; i < COLOR_NUMBER; i++) {
		f->color[i].r = (color[i] >> 16 & 0xff) / 255.0;
		f->color[i].g = (color[i] >>  8 & 0xff) / 255.0;
		f->color[i].b = (color[i] >>  0 & 0xff) / 255.0;
		f->color[i].a = (color[i] >> 24 & 0xff) / 255.0;
	}
}

static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  	struct flag *f;
	struct stock *s;
	const double degrees = M_PI / 180.0;
	cairo_surface_t *cs;
	char fn[256];
	double x, y;

	f = (struct flag *)data;
	g_assert(f);

	if (!f) {
		g_error(NULL);
	  	return FALSE;
	}

	cairo_scale(cr, f->scale, f->scale);
	cairo_translate(cr, STOCK_HEIGHT + 1, STOCK_HEIGHT + 1);
	s = &f->stock;
	flag_init(cr, &flag);

	/*if (f->width + f->border_width + s->height > DRAWING_AREA_WIDTH ||*/
	/*f->height + f->border_width + s->height > DRAWING_AREA_WIDTH) {*/
	/*g_error("image clipping!!!\n");*/
	/*}*/

	cairo_new_sub_path(cr);
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	cairo_set_source_rgba(cr, f->color[FLAG_COLOR].r,
	    f->color[FLAG_COLOR].g, f->color[FLAG_COLOR].b,
	    f->color[FLAG_COLOR].a);

	if (s->style == STOCK_LEFT_CENTER) {
		cairo_move_to(cr, -STOCK_HEIGHT, f->height / 2);
		cairo_line_to(cr, 0, 0);
	} else if (s->style == STOCK_LEFT_TOP) {
		cairo_move_to(cr, -STOCK_HEIGHT, 0);
	} else if (s->style == STOCK_LEFT_BOTTOM) {
		cairo_move_to(cr, -STOCK_HEIGHT, f->height);
		cairo_line_to(cr, 0, 0);
	} else {
		cairo_arc(cr, RADIUS, RADIUS, RADIUS, 180 * degrees,
		    270 * degrees);
	}

	if (s->style == STOCK_TOP_LEFT) {
		cairo_line_to(cr, RADIUS, -STOCK_HEIGHT);
		cairo_line_to(cr, RADIUS + STOCK_WIDTH, 0);
	}

	if (s->style == STOCK_TOP_CENTER) {
		cairo_line_to(cr, MARGIN_LR + (f->text_width - STOCK_WIDTH) / 2,
		    0);
		cairo_line_to(cr, MARGIN_LR + f->text_width / 2, -STOCK_HEIGHT);
		cairo_line_to(cr, MARGIN_LR + (f->text_width + STOCK_WIDTH) / 2,
		    0);
	}

	if (s->style == STOCK_TOP_RIGHT) {
		cairo_line_to(cr, f->width - RADIUS - STOCK_WIDTH, 0);
		cairo_line_to(cr, f->width - RADIUS, -STOCK_HEIGHT);
		cairo_line_to(cr, f->width - RADIUS, 0);
	}

	if (s->style == STOCK_RIGHT_CENTER) {
		cairo_line_to(cr, f->width, 0);
		cairo_line_to(cr, f->width + BORDER_WIDTH + STOCK_HEIGHT,
		    f->height / 2);
		cairo_line_to(cr, f->width, f->height);
	} else if (s->style == STOCK_RIGHT_TOP) {
		cairo_line_to(cr, f->width + BORDER_WIDTH + STOCK_HEIGHT, 0);
		cairo_line_to(cr, f->width, f->height);
	} else if (s->style == STOCK_RIGHT_BOTTOM) {
		cairo_line_to(cr, f->width, 0);
		cairo_line_to(cr, f->width + BORDER_WIDTH + STOCK_HEIGHT,
		    f->height);
	} else {
		cairo_arc(cr, f->width - RADIUS, RADIUS, RADIUS,
		    -90 * degrees, 0 * degrees);
		cairo_arc(cr, f->width - RADIUS, f->height - RADIUS,
		    RADIUS, 0 * degrees, 90 * degrees);
	}

	if (s->style == STOCK_BOTTOM_RIGHT) {
		cairo_line_to(cr, f->width - RADIUS, f->height + STOCK_HEIGHT);
		cairo_line_to(cr, f->width - RADIUS - STOCK_WIDTH, f->height);
	}

	if (s->style == STOCK_BOTTOM_CENTER) {
		cairo_line_to(cr, MARGIN_LR + (f->text_width + STOCK_WIDTH) / 2,
		    f->height);
		cairo_line_to(cr, MARGIN_LR + f->text_width / 2,
		    f->height + STOCK_HEIGHT);
		cairo_line_to(cr, MARGIN_LR + (f->text_width - STOCK_WIDTH) / 2,
		    f->height);
	}

	if (s->style == STOCK_BOTTOM_LEFT) {
		cairo_line_to(cr, RADIUS + STOCK_WIDTH, f->height);
		cairo_line_to(cr, RADIUS, f->height + STOCK_HEIGHT);
		cairo_line_to(cr, RADIUS, f->height);
	}

	if (s->style == STOCK_LEFT_CENTER || s->style == STOCK_LEFT_TOP) {
		cairo_line_to(cr, 0, f->height);
	} else if (s->style != STOCK_LEFT_BOTTOM) {
		cairo_arc(cr, RADIUS, f->height - RADIUS, RADIUS, 90 * degrees,
		    180 * degrees);
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
	cairo_move_to(cr, f->text_x, f->text_y);
	cairo_show_text(cr, f->text);
	cs = cairo_get_target(cr);
	x = f->width + BORDER_WIDTH + STOCK_HEIGHT + 1;
	y = f->height + BORDER_WIDTH + STOCK_HEIGHT + 1;
	cairo_user_to_device(cr, &x, &y);
	cs = cairo_surface_create_for_rectangle(cs, 0, 0, x, y);
	snprintf(fn, sizeof(fn), "%s_%s_%.1f.png", f->fnp,
	    stock_name[f->stock.style], f->scale);
	cairo_surface_write_to_png(cs, fn);

	return FALSE;
}

static void usage(void)
{
	printf("usage:\nmkfont [-t <\"text\"> -c <text color,flag color,"
	    "border color> -f <file name prefix> -s <scale>] [-h] [-V]\n"
	    "-t set text tag\n"
	    "-c set color AARRGGBB (in hex)\n"
	    "-f set file name prefix\n"
	    "-s set scale\n"
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
			gint V	: 1;
			gint h	: 1;
		} map;
		char *fnp;
		unsigned int color[COLOR_NUMBER];
		char *text;
		double scale;
	} param;
	GtkWidget *window;
	GtkWidget *drawing_area;

	g_log_set_handler(NULL, G_LOG_LEVEL_MASK, g_log_default_handler, NULL);
	memset(&param, 0, sizeof(param));

        while ((c = getopt(argc, argv, "t:c:f:s:hV")) != -1) {
		switch (c) {
		case 't':
			param.map.t = 1;
			param.text = optarg;
			g_debug("text tag %s\n", param.text);
			break;

		case 'c':
			if (sscanf(optarg, "%08x,%08x,%08x",
			    &param.color[TEXT_COLOR], &param.color[FLAG_COLOR],
			    &param.color[BORDER_COLOR]) == 3) {
				param.map.c = 1;
				g_debug("color text %08x flag %08x border "
				    "%08x\n", param.color[TEXT_COLOR],
				    param.color[FLAG_COLOR],
				    param.color[BORDER_COLOR]);
			}
			break;

		case 'f':
			param.map.f = 1;
			param.fnp = optarg;
			g_debug("file name prefix %s\n", param.fnp);
			break;

		case 's':
			param.map.s = 1;
			sscanf(optarg, "%lf", &param.scale);
			g_debug("scale %f\n", param.scale);
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
		flag_setup(&flag, STOCK_NONE, param.scale, param.text,
		    param.color, param.fnp);
		gtk_init(&argc, &argv);
		window = gtk_offscreen_window_new();
		drawing_area = gtk_drawing_area_new();
		gtk_widget_set_size_request(drawing_area, DRAWING_AREA_WIDTH,
		    DRAWING_AREA_HEIGHT);
		gtk_container_add(GTK_CONTAINER(window), drawing_area);
		g_signal_connect(G_OBJECT(drawing_area), "draw",
		    G_CALLBACK(draw_callback), &flag);
		gtk_widget_show_all(window);
		/*gtk_widget_queue_draw_area(window, 0, 0,*/
		/*DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT);*/
		/*gdk_window_process_updates(*/
		/*gtk_widget_get_window(window), TRUE);*/

		for (c = STOCK_LEFT_TOP; c < STOCK_NUM; c++) {
			flag_setup(&flag, c, param.scale, param.text,
					param.color, param.fnp);
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
