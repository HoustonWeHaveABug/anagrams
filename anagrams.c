#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define OPTIONS_MAX 256
#define BUFFER_SIZE OPTIONS_MAX+2
#define CHOICES_MAX OPTIONS_MAX*2-1

typedef struct letter_s letter_t;
typedef struct node_s node_t;
typedef struct option_s option_t;

struct letter_s {
	int symbol;
	node_t *next;
};

struct node_s {
	unsigned long letters_n;
	letter_t *letters;
};

struct option_s {
	int symbol;
	option_t *next;
	option_t *last;
};

node_t *new_node(void);
letter_t *new_letter(node_t *, int);
void set_letter(letter_t *, int);
void sort_node(node_t *);
int sort_letters(const void *, const void *);
int sort_options(const void *, const void *);
void link_option(option_t *, option_t *, option_t *);
void anagrams(node_t *, unsigned long, unsigned long);
void free_node(node_t *);

char buffer[BUFFER_SIZE];
unsigned long word_len_min, options_n, nodes_n, solutions_n;
letter_t *choices[CHOICES_MAX];
node_t *node_root;
option_t *option_header;

int main(int argc, char *argv[]) {
char *end;
int symbol;
unsigned long i, j;
FILE *fd;
letter_t *letter;
node_t *node;
option_t options[OPTIONS_MAX+1], *option;
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <path to dictionary> <word minimal length>\n", argv[0]);
		return EXIT_FAILURE;
	}
	node_root = new_node();
	if (!node_root) {
		return EXIT_FAILURE;
	}
	fd = fopen(argv[1], "r");
	if (!fd) {
		fprintf(stderr, "Could not open dictionary\n");
		free_node(node_root);
		return EXIT_FAILURE;
	}
	word_len_min = strtoul(argv[2], &end, 10);
	if (*end || !word_len_min) {
		fprintf(stderr, "Invalid word minimal length\n");
		fclose(fd);
		free_node(node_root);
		return EXIT_FAILURE;
	}
	while (fgets(buffer, BUFFER_SIZE, fd)) {
		node = node_root;
		for (i = 0; buffer[i] && buffer[i] != '\n'; i++);
		if (buffer[i] == '\n') {
			buffer[i] = '\0';
		}
		if (i >= word_len_min) {
			for (i = 0; buffer[i]; i++) {
				symbol = tolower((int)buffer[i]);
				if (islower(symbol)) {
					for (j = 0; j < node->letters_n && node->letters[j].symbol != symbol; j++);
					if (j < node->letters_n) {
						letter = node->letters+j;
					}
					else {
						letter = new_letter(node, symbol);
						if (!letter) {
							fclose(fd);
							free_node(node_root);
							return EXIT_FAILURE;
						}
					}
					node = letter->next;
					if (!node) {
						node = new_node();
						if (!node) {
							fclose(fd);
							free_node(node_root);
							return EXIT_FAILURE;
						}
						letter->next = node;
					}
				}
			}
			for (i = 0; i < node->letters_n && node->letters[i].symbol != ' '; i++);
			if (i == node->letters_n && !new_letter(node, ' ')) {
				fclose(fd);
				free_node(node_root);
				return EXIT_FAILURE;
			}
		}
	}
	fclose(fd);
	sort_node(node_root);
	while (fgets(buffer, BUFFER_SIZE, stdin)) {
		options_n = 0;
		for (i = 0; buffer[i] && buffer[i] != '\n'; i++) {
			symbol = tolower((int)buffer[i]);
			if (islower(symbol)) {
				options[options_n].symbol = symbol;
				options_n++;
			}
		}
		if (buffer[i] == '\n') {
			buffer[i] = '\0';
		}
		qsort(options, options_n, sizeof(option_t), sort_options);
		option_header = options+options_n;
		link_option(options, option_header, options+1);
		for (option = options+1; option < option_header; option++) {
			link_option(option, option-1, option+1);
		}
		link_option(option, option-1, options);
		nodes_n = 0;
		solutions_n = 0;
		anagrams(node_root, 0UL, 0UL);
		if (!solutions_n) {
			printf("%s -> ?\n", buffer);
		}
		printf("Nodes %lu\nSolutions %lu\n", nodes_n, solutions_n);
	}
	free_node(node_root);
	return EXIT_SUCCESS;
}

node_t *new_node(void) {
node_t *node;
	node = malloc(sizeof(node_t));
	if (!node) {
		fprintf(stderr, "Could not allocate memory for node\n");
		return NULL;
	}
	node->letters_n = 0;
	node->letters = NULL;
	return node;
}

letter_t *new_letter(node_t *node, int symbol) {
letter_t *letters;
	if (node->letters_n) {
		letters = realloc(node->letters, sizeof(letter_t)*(node->letters_n+1));
		if (!letters) {
			fprintf(stderr, "Could not reallocate memory for letters\n");
			free(node->letters);
			node->letters_n = 0;
			return NULL;
		}
		node->letters = letters;
	}
	else {
		node->letters = malloc(sizeof(letter_t));
		if (!node->letters) {
			fprintf(stderr, "Could not allocate memory for letters\n");
			return NULL;
		}
	}
	set_letter(node->letters+node->letters_n, symbol);
	node->letters_n++;
	return node->letters+node->letters_n-1;
}

void set_letter(letter_t *letter, int symbol) {
	letter->symbol = symbol;
	letter->next = NULL;
}

void sort_node(node_t *node) {
unsigned long i;
	if (node->letters_n) {
		qsort(node->letters, node->letters_n, sizeof(letter_t), sort_letters);
		for (i = 0; i < node->letters_n; i++) {
			if (node->letters[i].next) {
				sort_node(node->letters[i].next);
			}
		}
	}
}

int sort_letters(const void *a, const void *b) {
const letter_t *letter_a = (const letter_t *)a, *letter_b = (const letter_t *)b;
	return letter_a->symbol-letter_b->symbol;
}

int sort_options(const void *a, const void *b) {
const option_t *option_a = (const option_t *)a, *option_b = (const option_t *)b;
	return option_a->symbol-option_b->symbol;
}

void link_option(option_t *option, option_t *last, option_t *next) {
	option->last = last;
	option->next = next;
}

void anagrams(node_t *node, unsigned long option_idx, unsigned long choice_idx) {
unsigned long i;
option_t *option;
	nodes_n++;
	if (option_header->next == option_header) {
		if (node->letters_n && node->letters[0].symbol == ' ') {
			solutions_n++;
			if (solutions_n == 1) {
				printf("%s -> ", buffer);
				for (i = 0; i < choice_idx; i++) {
					putchar(choices[i]->symbol);
				}
				puts("");
			}
		}
	}
	else {
		if (node->letters_n) {
			if (node->letters[0].symbol == ' ') {
				i = 1;
			}
			else {
				i = 0;
			}
			option = option_header->next;
			while (i < node->letters_n) {
				while (option != option_header && option->symbol < node->letters[i].symbol) {
					option = option->next;
				}
				if (option != option_header && option->symbol == node->letters[i].symbol && node->letters[i].next) {
					option->last->next = option->next;
					option->next->last = option->last;
					choices[choice_idx] = node->letters+i;
					anagrams(node->letters[i].next, option_idx+1, choice_idx+1);
					option->next->last = option;
					option->last->next = option;
				}
				i++;
			}
			if (node->letters[0].symbol == ' ' && options_n-option_idx >= word_len_min) {
				choices[choice_idx] = node->letters;
				anagrams(node_root, option_idx, choice_idx+1);
			}
		}
	}
}

void free_node(node_t *node) {
unsigned long i;
	if (node->letters_n) {
		for (i = 0; i < node->letters_n; i++) {
			if (node->letters[i].next) {
				free_node(node->letters[i].next);
			}
		}
		free(node->letters);
	}
	free(node);
}
