/* Copyright 2023 <Dragan Dragos Ovidiu> */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "load_balancer.h"

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

// fac lista cu serverele;
load_balancer *init_load_balancer()
{

	load_balancer *circle;
	circle = malloc(sizeof(load_balancer));
	circle->size = 0;
	circle->vector_servere = malloc(sizeof(servere *));

	return circle;
}

int introducere_mijloc(load_balancer *main, int server_id, unsigned int hash, int k, int *i)
{
	for (*i = 0; *i < main->size - 1; (*i)++) { // se afla la mijloc
		if (main->vector_servere[*i]->server_hash < hash && hash < main->vector_servere[(*i) + 1]->server_hash) {
			for (int j = main->size; j >= (*i) + 1; j--) {// translatez toti vectorii o pozitie mai la dreapta
				main->vector_servere[j] = main->vector_servere[j - 1];
			}
			main->vector_servere[(*i) + 1] = malloc(sizeof(servere));
			main->vector_servere[(*i) + 1]->data = init_server_memory();
			main->vector_servere[(*i) + 1]->ID = 100000 * k + server_id;
			main->vector_servere[(*i) + 1]->server_hash = hash;
			main->size++;
			return 1;
		}
	}
	return 0;
}

void realocare_chei(load_balancer *main, int index, int index_curent, unsigned int hash)
{
	// hash = hashul serverului meu in care bag
	unsigned int i, j;
	// if (main->vector_servere[index]->ID == 41469) {
	// 	for (int i = 0; i < main->vector_servere[index]->data->hmax; i++) {
	// 		linked_list_t *list = main->vector_servere[index]->data->buckets[i];
	// 		ll_node_t *curr = list->head;
	// 		for (j = 0; j < list->size; j++) {
	// 			printf("%s\n", (char *)((info *)curr->data)->value);
	// 		}
	// 	}
	// }
	for (i = 0; i < main->vector_servere[index]->data->hmax; i++) { //parcurg tot hastable ul meu
		if (main->vector_servere[index]->data->buckets[i]->head) { // daca headul meu este diferit de NULL
			ll_node_t *curr = main->vector_servere[index]->data->buckets[i]->head; // parcurg toata lista mea de noduri
			for (j = 0; j < main->vector_servere[index]->data->buckets[i]->size; j++) { // parcurg fiecare cheie din lista de bucketuri
				unsigned int key_hash = hash_function_key(((info *)curr->data)->key);
				ll_node_t *aux = curr->next;
				if (index == 0 && index_curent == main->size - 1) { // daca bag in ultimul si caut in primul
					if (key_hash < hash && key_hash > main->vector_servere[main->size - 2]->server_hash) {
						// if (main->vector_servere[index_curent]->ID == 30986)
						// 	printf("\n\nplus %s realoc1\n\n", (char *)((info *)curr->data)->value);
						// if (main->vector_servere[index]->ID == 30986)
						// 	printf("\n\nminus %s\n\n", (char *)((info *)curr->data)->value);
						server_store(main->vector_servere[index_curent]->data, ((info *)curr->data)->key, ((info *)curr->data)->value);
						server_remove(main->vector_servere[index]->data, ((info *)curr->data)->key);
					}
					continue;
				}
				if (index == 1 && index_curent == 0) { // daca bag inainte de primul nod si caut in primul
					if (key_hash < hash || key_hash > main->vector_servere[main->size - 1]->server_hash) {
						// if (main->vector_servere[index_curent]->ID == 30986)
						// 	printf("\n\nplus %s realoc2\n\n", (char *)((info *)curr->data)->value);
						// if (main->vector_servere[index]->ID == 30986)
						// 	printf("\n\nminus %s\n\n", (char *)((info *)curr->data)->value);
						server_store(main->vector_servere[index_curent]->data, ((info *)curr->data)->key, ((info *)curr->data)->value);
						server_remove(main->vector_servere[index]->data, ((info *)curr->data)->key);
					}
					continue;
				}

				if (key_hash < hash) { // daca e la mijloc
					// if (main->vector_servere[index_curent]->ID == 30986)
					// 	printf("\n\nplus %s realoc3\n\n", (char *)((info *)curr->data)->value);
					// if (main->vector_servere[index]->ID == 30986)
					// 	printf("\n\nminus %s\n\n", (char *)((info *)curr->data)->value);
					server_store(main->vector_servere[index_curent]->data, ((info *)curr->data)->key, ((info *)curr->data)->value);
					server_remove(main->vector_servere[index]->data, ((info *)curr->data)->key);
				}
				curr = aux;
			}
		}
	}
}

void introducere_final_inceput(load_balancer *main, int server_id, unsigned int hash, int k)
{
	//printf("%d\n%d\n%d\n\n", hash, main->vector_servere[0]->server_hash, main->vector_servere[main->size - 1]->server_hash);
	if (hash < main->vector_servere[0]->server_hash) { // inserez pe prima pozitie
		for (int j = main->size; j > 0; j--)
			main->vector_servere[j] = main->vector_servere[j - 1];

		main->vector_servere[0] = malloc(sizeof(servere));
		main->vector_servere[0]->data = init_server_memory();
		main->vector_servere[0]->ID = 100000 * k + server_id;
		main->vector_servere[0]->server_hash = hash;
		main->size++;
		realocare_chei(main, 1, 0, hash);
	} else if (hash > main->vector_servere[main->size - 1]->server_hash) { // e pe ultima pozitie
		main->vector_servere[main->size] = malloc(sizeof(servere));
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
	hash = hash_function_servers(&hash); // hashul serverului
	int k = 0;
	main->vector_servere = realloc(main->vector_servere, (main->size + 3) * sizeof(servere));

	if (main->size == 0) { // vectorul meu este gol
		main->vector_servere[0] = malloc(sizeof(servere));
		main->vector_servere[0]->data = init_server_memory();
		main->vector_servere[0]->ID = server_id;
		main->vector_servere[0]->server_hash = hash;
		main->size++;
		k = 1;
		//printf("%d\n\n\n%d\n\n", main->vector_servere[0]->server_hash, server_id);
	}
	int i = 0;
	if (k == 0) {// daca e prima oara cand ii fac hashul si sa vad unde l bag
		int ok = introducere_mijloc(main, server_id, hash, 0, &i);

		if (ok == 0) // ori e dupa ori e inainte
			introducere_final_inceput(main, server_id, hash, 0);
		else
			realocare_chei(main, i + 2, i + 1, hash);

		k++;
	}

	i = 0;
	if (k == 1) {
		unsigned int replica = 100000 + server_id;
		hash = hash_function_servers(&replica);
		int ok = introducere_mijloc(main, server_id, hash, 1, &i);
		//printf("%d", ok);
		if (ok == 0)  // ori e dupa ori e inainte
			introducere_final_inceput(main, server_id, hash, 1);
		else
			realocare_chei(main, i + 2, i + 1, hash);

		k++;
	}

	i = 0;
	if (k == 2) {
		unsigned int replica = 200000 + server_id;
		hash = hash_function_servers(&replica);
		int ok = introducere_mijloc(main, server_id, hash, 2, &i);

		if (ok == 0) // ori e dupa ori e inainte
			introducere_final_inceput(main, server_id, hash, 2);
		else
			realocare_chei(main, i + 2, i + 1, hash);

		k++;
	}
	// for (int i = 0; i < main->size; i++) {
	// 	printf("%u ", main->vector_servere[i]->ID);
	// }
	// printf("\n");
	// for (int i = 0; i < main->size; i++) {
	// 	printf("%u ", main->vector_servere[i]->server_hash);
	// }
	// printf("\n");

}

int verficare_server(load_balancer *main, int server_id)
{
	unsigned int hash = hash_function_servers(&server_id);

	for (int i = 0; i < main->size; i++)
		if (main->vector_servere[i]->server_hash == hash)
			return i;

	printf("Nu a mers");
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

		// eu am nevoie de server id ul urmatorului
		//unsigned int hash = main->vector_servere[index_next]->server_hash;
		// serverul care detine serverid ul
		//realocare_chei(main, index, index_next, hash);
		for (unsigned int i = 0; i < main->vector_servere[index]->data->hmax; i++) { //parcurg tot hastable ul meu
			if (main->vector_servere[index]->data->buckets[i]->head) { // daca headul meu este diferit de NULL
				ll_node_t *curr = main->vector_servere[index]->data->buckets[i]->head; // parcurg toata lista mea de noduri
				for (unsigned int j = 0; j < main->vector_servere[index]->data->buckets[i]->size; j++) { // parcurg fiecare cheie din lista de bucketuri
					// if (main->vector_servere[index_next]->ID == 30986)
					// 	printf("\n\nplus %s mutare_next\n\n", (char *)((info *)curr->data)->value);
					server_store(main->vector_servere[index_next]->data, ((info *)curr->data)->key, ((info *)curr->data)->value);
					//server_remove(main->vector_servere[index]->data, ((info *)curr->data)->key);
					curr = curr->next;
				}
			}
		}
		// if (main->vector_servere[index]->ID == 30986)
		// 	printf("\n\n0\n\n");
		free_server_memory(main->vector_servere[index]->data);
		free(main->vector_servere[index]);

		for (int i = index; i < main->size - 1; i++)
			main->vector_servere[i] = main->vector_servere[i + 1];

		main->size--;
	}
	main->vector_servere = realloc(main->vector_servere, (main->size) * sizeof(servere));
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	unsigned int hash_key = hash_function_key(key);
	//	printf("    %u\n\n\n", hash_key);
	if (hash_key > main->vector_servere[main->size - 1]->server_hash) { // e pe ultima pozitie
		// if (strncmp(value, "Vaishali", 8) == 0) {
		// 	printf("\n\nplus %s store1\n\n", main->vector_servere[0]->ID);
		// }

		// if (main->vector_servere[0]->ID == 30986)
		// 	printf("\n\nplus %s store1\n\n", value);
		server_store(main->vector_servere[0]->data, key, value);
		*server_id = main->vector_servere[0]->ID % 100000;
		//	printf("%d", main->vector_servere[0]->ID);
		return;
	}

	for (int i = 0; i < main->size; i++) {
		if (main->vector_servere[i]->server_hash > hash_key) {
			// if (strncmp(value, "Vaishali", 8) == 0) {
			// 	printf("\n\nplus %s store2\n\n", main->vector_servere[i]->ID);
			// }
			// if (main->vector_servere[i]->ID == 30986)
			// 	printf("\n\nplus %s store2\n\n", value);
			server_store(main->vector_servere[i]->data, key, value);
			*server_id = main->vector_servere[i]->ID % 100000;
			return;
		}
	}
}

// intoarce serverul in care exista cheia
char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	unsigned int hash_key = hash_function_key(key); // sa vad unde se afla cheia mea

	for (int i = 0; i < main->size - 1; i++) {
		if (main->vector_servere[i]->server_hash < hash_key && hash_key < main->vector_servere[i + 1]->server_hash) {
			char *value = server_retrieve(main->vector_servere[i + 1]->data, key);
			if (value) {
				*server_id = main->vector_servere[i + 1]->ID % 100000;
				return value;
			} else return NULL;
		}
	}
	// ori e dupa ori e inainte
	if (hash_key < main->vector_servere[0]->server_hash) { // cheia se afla inainte primului sensor
		char *value = server_retrieve(main->vector_servere[0]->data, key);
		if (value) {
			*server_id = main->vector_servere[0]->ID % 100000;
			return value;
		} else return NULL;
	} else if (hash_key > main->vector_servere[main->size - 1]->server_hash) { // e dupa ultima pozitie
		char *value = server_retrieve(main->vector_servere[0]->data, key);
		if (value) {
			*server_id = main->vector_servere[0]->ID % 100000;
			return value;
		} else return NULL;
	}

	return NULL;
}

void free_load_balancer(load_balancer *main)
{
	for (int i = 0; i < main->size; i++) {
		//printf("%d %d\n", main->vector_servere[i]->ID, main->vector_servere[i]->data->size);
		free_server_memory(main->vector_servere[i]->data);
		free(main->vector_servere[i]);
	}
	free(main->vector_servere);
	free(main);
}
