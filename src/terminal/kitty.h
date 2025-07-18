#ifndef EL__TERMINAL_KITTY_H
#define EL__TERMINAL_KITTY_H

#include "util/lists.h"
#include "util/string.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_KITTY
struct document;
struct el_box;
struct terminal;

struct el_string {
	char *data;
	unsigned int length;
	int refcnt;
};

struct k_image {
	LIST_HEAD_EL(struct k_image);
	struct el_string *pixels;
	int cx;
	int cy;
	int width;
	int height;
	int id;
	int ID;
	int number;
	int x;
	int y;
	int w;
	int h;
	unsigned int sent:1;
	unsigned int compressed:1;
};

struct el_string *el_string_init(char *data, unsigned int length);
struct el_string *el_string_ref(struct el_string *el_string);
void el_string_unref(struct el_string *el_string);

void delete_k_image(struct k_image *im);

void try_to_draw_k_images(struct terminal *term, struct string *text);

/* return height of image in terminal lines */
int add_kitty_image_to_document(struct document *doc, struct el_string *pixels, int lineno, struct k_image **imagine, int width, int height);

struct k_image *copy_k_frame(struct k_image *src, struct el_box *box, int cell_width, int cell_height, int dx, int dy);

#endif

#ifdef __cplusplus
}
#endif

#endif /* EL__TERMINAL_KITTY_H */
