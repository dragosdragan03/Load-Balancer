/* Copyright 2023 <Dragan Dragos Ovidiu> */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "/home/student/Desktop/de_predat/SDA/Tema 2/Load-Balancer/load_balancer.h"
#include "/home/student/Desktop/de_predat/SDA/Tema 2/Load-Balancer/utils.h"

typedef struct servere {
	server_memory *data;
	unsigned int server_hash;  // hasul serverului
	unsigned int ID;
}servere;

struct load_balancer {
	servere **vector_servere;  // vector de pointeri
	int size;
};

unsigned int hash_function_servers(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

load_balancer *init_load_balancer()
{
	load_balancer *circle;
	circle = malloc(sizeof(load_balancer));
	DIE(!circle, "Nu s-a putut aloca");
	circle->size = 0;
	circle->vector_servere = malloc(sizeof(servere *));
	DIE(!circle->vector_servere, "Nu s-a putut aloca");

	return circle;
}

int introducere_mijloc(load_balancer *main, int server_id, unsigned int hash
	, int k, int *i)
{
	for (*i = 0; *i < main->size - 1; (*i)++) {  // se afla la mijloc
		if (main->vector_servere[*i]->server_hash < hash &&
			hash < main->vector_servere[(*i) + 1]->server_hash) {
			// translatez toti vectorii o pozitie mai la dreapta
			for (int j = main->size; j >= (*i) + 1; j--) {
				main->vector_servere[j] = main->vector_servere[j - 1];
			}
			main->vector_servere[(*i) + 1] = malloc(sizeof(servere));
			DIE(!main->vector_servere[(*i) + 1], "Nu s-a putut aloca");
			main->vector_servere[(*i) + 1]->data = init_server_memory();
			main->vector_servere[(*i) + 1]->ID = 100000 * k + server_id;
			main->vector_servere[(*i) + 1]->server_hash = hash;
			main->size++;
			return 1;
		}
	}
	return 0;
}

void realocare_chei(load_balancer *main, int index, int index_curent
	, unsigned int hash)
{
	unsigned int i, j;
	// parcurg tot hastable ul meu
	for (i = 0; i < main->vector_servere[index]->data->hmax; i++) {
		ll_node_t *curr = main->vector_servere[index]->data->buckets[i]->head;

		// parcurg fiecare cheie din lista de bucketuri
		for (j = 0; j < main->vector_servere[index]->data->buckets[i]->size; j++) {
			char *cheie = ((info *)curr->data)->key;
			char *valoare = ((info *)curr->data)->value;
			unsigned int key_hash = hash_function_key(cheie);
			ll_node_t *aux = curr->next;
			server_memory *server = main->vector_servere[index_curent]->data;
			servere **vector = main->vector_servere;
			// daca bag in ultimul si caut in primul
			if (index == 0 && index_curent == main->size - 1) {
				if (key_hash < hash && key_hash > vector[main->size - 2]->server_hash) {
					server_store(server, cheie, valoare);
					server_remove(vector[index]->data, cheie);
				}
				continue;
			}
			// daca bag inainte de primul nod si caut in primul
			if (index == 1 && index_curent == 0) {
				if (key_hash < hash ||
					key_hash > vector[main->size - 1]->server_hash) {
					server_store(server, cheie, valoare);
					server_remove(vector[index]->data, cheie);
				}
				continue;
			}

			if (key_hash < hash) {  // daca e la mijloc
				server_store(server, cheie, valoare);
				server_remove(main->vector_servere[index]->data, cheie);
			}
			curr = aux;
		}
	}
}

void introducere_final_inceput(load_balancer *main, int server_id
	, unsigned int hash, int k)
{
	// inserez pe prima pozitie
	if (hash < main->vector_servere[0]->server_hash) {
		for (int j = main->size; j > 0; j--)
			main->vector_servere[j] = main->vector_servere[j - 1];

		main->vector_servere[0] = malloc(sizeof(servere));
		DIE(!main->vector_servere[0], "Nu s-a putut aloca");
		main->vector_servere[0]->data = init_server_memory();
		main->vector_servere[0]->ID = 100000 * k + server_id;
		main->vector_servere[0]->server_hash = hash;
		main->size++;
		realocare_chei(main, 1, 0, hash);
		// e pe ultima pozitie
	} else if (hash > main->vector_servere[main->size - 1]->server_hash) {
		main->vector_servere[main->size] = malloc(sizeof(servere));
		DIE(!main->vector_servere[main->size], "Nu s-a putut aloca");
		main->vector_servere[main->size]->data = init_server_memory();
		main->vector_servere[main->size]->ID = 100000 * k + server_id;
		main->vector_servere[main->size]->server_hash = hash;
		main->size++;
		realocare_chei(main, 0, main->size - 1, hash);
	}
}

void loader_add_server(load_balancer *main, int server_id)
{
	unsigned int hash = server_id;
	hash = hash_function_servers(&hash);  // hashul serverului
	int k = 0;
	main->vector_servere = realloc(main->vector_servere,
		(main->size + 3) * sizeof(servere));
	DIE(!main->vector_servere, "Nu s-a putut aloca");

	if (main->size == 0) {  // vectorul meu este gol
		main->vector_servere[0] = malloc(sizeof(servere));
		DIE(!main->vector_servere[0], "Nu s-a putut aloca");
		main->vector_servere[0]->data = init_server_memory();
		main->vector_servere[0]->ID = server_id;
		main->vector_servere[0]->server_hash = hash;
		main->size++;
		k = 1;
	}
	int i = 0;
	if (k == 0) {  // serverul original
		int ok = introducere_mijloc(main, server_id, hash, 0, &i);

		if (ok == 0)  // ori e dupa ori e inainte
			introducere_final_inceput(main, server_id, hash, 0);
		else
			realocare_chei(main, i + 2, i + 1, hash);

		k++;
	}

	i = 0;
	if (k == 1) {  // prima replica
		unsigned int replica = 100000 + server_id;
		hash = hash_function_servers(&replica);
		int ok = introducere_mijloc(main, server_id, hash, 1, &i);

		if (ok == 0)  // ori e dupa ori e inainte
			introducere_final_inceput(main, server_id, hash, 1);
		else
			realocare_chei(main, i + 2, i + 1, hash);

		k++;
	}

	i = 0;
	if (k == 2) {  // a doua replica
		unsigned int replica = 200000 + server_id;
		hash = hash_function_servers(&replica);
		int ok = introducere_mijloc(main, server_id, hash, 2, &i);

		if (ok == 0)  // ori e dupa ori e inainte
			introducere_final_inceput(main, server_id, hash, 2);
		else
			realocare_chei(main, i + 2, i + 1, hash);

		k++;
	}
}

int verficare_server(load_balancer *main, int server_id)
{
	unsigned int hash = hash_function_servers(&server_id);

	for (int i = 0; i < main->size; i++)
		if (main->vector_servere[i]->server_hash == hash)
			return i;

	printf("Nu s-a gasit");
	return -1;
}

void loader_remove_server(load_balancer *main, int server_id)
{
	for (int k = 0; k < 3; k++) {
		int index = verficare_server(main, 100000 * k + server_id);
		int index_next = index + 1;
		if (index == -1)
			return;

		if (index + 1 == main->size)
			index_next = 0;

		// parcurg tot hastable ul meu
		unsigned int hmax = main->vector_servere[index]->data->hmax;
		for (unsigned int i = 0; i < hmax; i++) {
			if (main->vector_servere[index]->data->buckets[i]->head) {
				// parcurg toata lista mea de noduri
				server_memory *server = main->vector_servere[index]->data;
				ll_node_t *curr = server->buckets[i]->head;
				// parcurg fiecare cheie din lista de bucketuri
				for (unsigned int j = 0; j < server->buckets[i]->size; j++) {
					char *cheie = ((info *)curr->data)->key;
					char *valoare = ((info *)curr->data)->value;
					server_store(main->vector_servere[index_next]->data,
						cheie, valoare);
					curr = curr->next;
				}
			}
		}
		free_server_memory(main->vector_servere[index]->data);
		free(main->vector_servere[index]);

		for (int i = index; i < main->size - 1; i++)
			main->vector_servere[i] = main->vector_servere[i + 1];

		main->size--;
	}
	main->vector_servere = realloc(main->vector_servere,
		(main->size) * sizeof(servere));
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	unsigned int hash_key = hash_function_key(key);
	// e pe ultima pozitie
	if (hash_key > main->vector_servere[main->size - 1]->server_hash) {
		server_store(main->vector_servere[0]->data, key, value);
		*server_id = main->vector_servere[0]->ID % 100000;
		return;
	}

	for (int i = 0; i < main->size; i++) {
		if (main->vector_servere[i]->server_hash > hash_key) {
			server_store(main->vector_servere[i]->data, key, value);
			*server_id = main->vector_servere[i]->ID % 100000;
			return;
		}
	}
}

// intoarce serverul in care exista cheia
char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	unsigned int hash_key = hash_function_key(key);

	for (int i = 0; i < main->size - 1; i++) {
		if (main->vector_servere[i]->server_hash < hash_key &&
			hash_key < main->vector_servere[i + 1]->server_hash) {
			char *value = server_retrieve(main->vector_servere[i + 1]->data, key);
			if (value) {
				*server_id = main->vector_servere[i + 1]->ID % 100000;
				return value;
			} else {
				return NULL;
			}
		}
	}
	// cheia se afla inainte primului sensor
	if (hash_key < main->vector_servere[0]->server_hash) {
		char *value = server_retrieve(main->vector_servere[0]->data, key);
		if (value) {
			*server_id = main->vector_servere[0]->ID % 100000;
			return value;
		} else {
			return NULL;
		}
		// e dupa ultima pozitie
	} else {
		if (hash_key > main->vector_servere[main->size - 1]->server_hash) {
			char *value = server_retrieve(main->vector_servere[0]->data, key);
			if (value) {
				*server_id = main->vector_servere[0]->ID % 100000;
				return value;
			} else {
				return NULL;
			}
		}
	}

	return NULL;
}

void free_load_balancer(load_balancer *main)
{
	for (int i = 0; i < main->size; i++) {
		free_server_memory(main->vector_servere[i]->data);
		free(main->vector_servere[i]);
	}
	free(main->vector_servere);
	free(main);
}
