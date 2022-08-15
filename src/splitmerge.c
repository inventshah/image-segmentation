#include "utils.h"

#ifndef NULL
#define NULL ((void *)0)
#endif
#define HOMOGENEOUS 1
#define NOT_HOMOGENEOUS 0

struct node {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    struct node *next;
};
typedef struct node Node;

struct group {
    int32_t red;
    int32_t green;
    int32_t blue;
    int32_t count;
    Node *root;
    struct group *next;
};
typedef struct group Group;

int is_node_homogeneous(Node *node, int tolerance, Image img) {
    if (node->width == 0 || node->height == 0)
        return HOMOGENEOUS;

    int32_t upper_x = node->x + node->width;
    int32_t upper_y = node->y + node->height;
    int32_t red = 0, green = 0, blue = 0, count = node->width * node->height;
    int32_t min_distance = 1 << 30, max_distance = 0;

    for (int32_t x = node->x; x < upper_x && x < img.width; x++) {
        for (int32_t y = node->y; y < upper_y && y < img.height; y++) {
            uint8_t *rgb = img_get(img, x, y);

            int32_t current_distance =
                sqrt(rgb[0] * rgb[0] + rgb[1] * rgb[1] + rgb[2] * rgb[2]);

            red += (rgb[0]);
            green += (rgb[1]);
            blue += (rgb[2]);

            if (min_distance > current_distance)
                min_distance = current_distance;
            if (max_distance < current_distance)
                max_distance = current_distance;
        }
    }

    node->red = red / count;
    node->green = green / count;
    node->blue = blue / count;

    return max_distance - min_distance < tolerance ? HOMOGENEOUS
                                                   : NOT_HOMOGENEOUS;
}

Node *split_node(Node *node, int32_t tolerance, Image img);

Node *add_node(Node *parent, int32_t x, int32_t y, int32_t width,
               int32_t height, int32_t tolerance, Image img) {
    Node *ptr = (Node *)malloc(sizeof(Node));

    ptr->x = x;
    ptr->y = y;
    ptr->width = width;
    ptr->height = height;
    ptr->next = NULL;

    if (is_node_homogeneous(ptr, tolerance, img) == NOT_HOMOGENEOUS) {
        Node *temp = ptr;
        ptr = split_node(ptr, tolerance, img);
        parent->next = temp->next;
        free(temp);
    } else {
        parent->next = ptr;
    }

    return ptr;
}

Node *split_node(Node *node, int32_t tolerance, Image img) {
    int32_t width = node->width / 2;
    int32_t height = node->height / 2;

    int32_t offset_x = node->width & 1;
    int32_t offset_y = node->height & 1;

    int32_t x = node->x, y = node->y;
    node = add_node(node, x, y, width, height, tolerance, img);
    node =
        add_node(node, x + width, y, width + offset_x, height, tolerance, img);
    node =
        add_node(node, x, y + height, width, height + offset_y, tolerance, img);
    node = add_node(node, x + width, y + height, width + offset_x,
                    height + offset_y, tolerance, img);

    return node;
}

Group *new_group(Group *last, Node *node) {
    Group *ptr = (Group *)malloc(sizeof(Group));

    ptr->red = node->red;
    ptr->green = node->green;
    ptr->blue = node->blue;
    ptr->count = node->width * node->height;
    if (ptr->count == 0)
        ptr->count = 1;
    ptr->root = node;
    node->next = 0;
    ptr->next = 0;

    if (last != NULL)
        last->next = ptr;

    return ptr;
}

int touching(Node *one, Node *two) {
    if (one->y == two->y || one->y == two->height + two->y) {
        return ((one->x >= two->x && one->x <= two->x + two->width) ||
                (two->x >= one->x && two->x <= one->x + one->width));
    }
    if (one->x == two->x || one->x == two->width + two->x) {
        return ((one->y >= two->y && one->y <= two->y + two->height) ||
                (two->y >= one->y && two->y <= one->y + one->height));
    }

    return 0;
}

int32_t element_distance(Node *element, Group *group) {
    Node *current = group->root;
    int32_t rdiff, gdiff, bdiff;

    rdiff = element->red - group->red;
    gdiff = element->green - group->green;
    bdiff = element->blue - group->blue;

    rdiff *= rdiff;
    gdiff *= gdiff;
    bdiff *= bdiff;

    while (current != NULL) {
        if (touching(current, element) || touching(element, current))
            break;
        current = current->next;
    }

    if (current == NULL)
        return 1 << 30;

    return sqrt(rdiff + gdiff + bdiff);
}

void add_element_to_group(Group *group, Node *node) {
    int32_t count = node->width * node->height;

    node->next = group->root;
    group->root = node;

    group->red *= group->count;
    group->green *= group->count;
    group->blue *= group->count;

    group->red += node->red * count;
    group->green += node->green * count;
    group->blue += node->blue * count;

    group->count += count;
    group->red /= group->count;
    group->green /= group->count;
    group->blue /= group->count;
}

void colorize_groups(Image img, Group *group) {
    for (Node *node = group->root; node != NULL; node = node->next) {
        int32_t upper_x = node->x + node->width;
        int32_t upper_y = node->y + node->height;

        for (int32_t x = node->x; x < upper_x; x++) {
            for (int32_t y = node->y; y < upper_y; y++) {
                uint8_t red = (group->red);
                uint8_t green = (group->green);
                uint8_t blue = (group->blue);
                img_set(img, x, y, red, green, blue, 255);
            }
        }
    }
}

void colorize_split(Node *node, Image img) {
    for (; node != NULL; node = node->next) {
        int32_t upper_x = node->x + node->width;
        int32_t upper_y = node->y + node->height;

        for (int32_t x = node->x; x < upper_x; x++) {
            for (int32_t y = node->y; y < upper_y; y++) {
                img_set(img, x, y, node->red, node->green, node->blue, 255);
            }
        }
    }
}

void free_groups(Group *group) {
    if (group == NULL)
        return;

    for (Node *p = group->root; p != NULL;) {
        Node *next = p->next;
        free(p);
        p = next;
    }

    free_groups(group->next);
    free(group);
}

void merge(Node *root, int32_t tolerance, Image img) {
    Node *current = root->next;
    Group *first_group, *last_group, *current_group = NULL;
    first_group = last_group = new_group(0, root);

    while (current != NULL) {
        for (current_group = first_group; current_group != NULL;
             current_group = current_group->next) {
            if (element_distance(current, current_group) < tolerance)
                break;
        }

        Node *next = current->next;
        if (current_group == NULL) {
            last_group = new_group(last_group, current);
        } else {
            add_element_to_group(current_group, current);
        }
        current = next;
    }

    for (current_group = first_group; current_group != NULL;
         current_group = current_group->next) {
        colorize_groups(img, current_group);
    }

    free_groups(first_group);
}

EXPORT("split_and_merge")
void split_and_merge(uint8_t *data, int32_t width, int32_t height,
                     int32_t tolerance) {
    Image img = {data, width, height};

    img_gamma_correct(img);
    Node head;
    add_node(&head, 0, 0, img.width, img.height, tolerance, img);
    merge(head.next, tolerance / 2, img);
    img_gamma_reset(img);
}
