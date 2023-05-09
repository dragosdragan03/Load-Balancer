/* Copyright 2023 <Dragan Dragos Ovidiu> */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "server.h"
//trebuie sa mi implementez functii de lista simplu inlanuite, de hash
//

unsigned int hash_function_k(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

linked_list_t *ll_create(unsigned int data_size)
{
	linked_list_t *list = malloc(sizeof(linked_list_t));

	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;

	return list;
}

// ll_node_t *create_node(void *new_data, int data_size) // creez un nod
// {
// 	// aloc diamic un nod
// 	ll_node_t *new_node = malloc(sizeof(ll_node_t));

// 	// aloca spatiu pentru tipul de data adaugat
// 	new_node->data = malloc(data_size);

// 	// copiaza in new_node, ce contine new_data cu size ul respectiv
// 	memcpy(new_node->data, new_data, data_size);

// 	return new_node;
// }

ll_node_t *ll_remove_nth_node(linked_list_t *list, unsigned int n)
{
	ll_node_t *prev, *curr;

	if (!list || !list->head) {
		return NULL;
	}

	/* n >= list->size - 1 inseamna eliminarea nodului de la finalul listei. */
	if (n > list->size - 1) {
		n = list->size - 1;
	}

	curr = list->head;
	prev = NULL;
	while (n > 0) {
		prev = curr;
		curr = curr->next;
		--n;
	}

	if (prev == NULL) {
		/* Adica n == 0. */
		list->head = curr->next;
	} else {
		prev->next = curr->next;
	}

	list->size--;

	return curr;
}

void ll_add_nth_node(linked_list_t *list, unsigned int n, const void *new_data)
{
	ll_node_t *prev, *curr;
	ll_node_t *new_node;

	if (!list) {
		return;
	}
	if (n > list->size) {
		n = list->size;
	}
	curr = list->head;
	prev = NULL;
	while (n > 0) {
		prev = curr;
		curr = curr->next;
		--n;
	}
	new_node = malloc(sizeof(*new_node));
	new_node->data = malloc(list->data_size);
	memcpy(new_node->data, new_data, list->data_size);
	new_node->next = curr;
	if (prev == NULL) {
		list->head = new_node;
	} else {
		prev->next = new_node;
	}
	list->size++;
}

void key_val_free_function(void *data)
{
	free(((info *)data)->key);
	free(((info *)data)->value);
	free(data);
}

void ll_free(linked_list_t **pp_list)
{
	linked_list_t *list = *pp_list;

	if (list->size > 0) {
		ll_node_t *curr = list->head;
		ll_node_t *aux;
		for (unsigned int i = 0; i < list->size; i++) {
			aux = curr->next;
			key_val_free_function(curr->data); // eliberez memoria din noduri
			free(curr); // eliberez nodul
			curr = aux;
		}
	}
	free(*pp_list); // sterg lista
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

server_memory *init_server_memory()
{
	server_memory *server;
	server = malloc(sizeof(server_memory));
	server->hmax = 9999;
	server->size = 0;
	server->hash_function = hash_function_k;
	server->key_val_free_function = key_val_free_function;
	server->compare_function = compare_function_strings;
	server->buckets = malloc(server->hmax * sizeof(linked_list_t *));
	for (int i = 0; i < server->hmax; i++)
		server->buckets[i] = ll_create(sizeof(info));

	return server;

}

// o sa primesc memoria unui server (ex: key, value)
void server_store(server_memory *server, char *key, char *value)
{
	unsigned int hash = server->hash_function(key);
	int index = hash % server->hmax;
	ll_node_t *curr = server->buckets[index]->head;
	// if (!strncmp(value, "Vaishali Bindi and Bangles Brass, Copper Bangle Set", sizeof("Vaishali Bindi and Bangles Brass, Copper Bangle Set"))) {
	// 	printf("************\n%d\n*****************", index);
	// }
	if (server_retrieve(server, key)) { // daca exista cheia
		for (unsigned int i = 0; i < server->buckets[index]->size; i++) {
			if (!server->compare_function(key, ((info *)curr->data)->key)) {
				//printf("%s", (char *)((info *)curr->data)->key);
				//printf("\n\n\n\n*************************************\n\n\n");
				break;
			}
			curr = curr->next;
		}
		memcpy(((info *)curr->data)->value, value, 65536);
	} else {
		info *cop = malloc(sizeof(info));
		cop->key = malloc(128);
		cop->value = malloc(65536);
		memcpy(cop->key, key, 128);
		memcpy(cop->value, value, 65536);
		ll_add_nth_node(server->buckets[index], server->buckets[index]->size, cop);
		// if (!strncmp(value, "Vaishali Bindi and Bangles Brass, Copper Bangle Set", sizeof("Vaishali Bindi and Bangles Brass, Copper Bangle Set"))) {
		// 	printf("************\n%d\n*****************", server->buckets[index]->size);
		// 	ll_node_t *curr = server->buckets[index]->head;
		// 	for (int i = 0; i < server->buckets[index]->size; i++) {
		// 		printf("%s\n", (char *)((info *)curr->data)->value);
		// 		curr = curr->next;
		// 	}
		// }
		server->size++;
		//printf("%s", (char *)cop->key);
		free(cop);
	}
}
// mi se da o cheie si eu vreau sa i aflu valoarea ei si mi returneaza valoarea
char *server_retrieve(server_memory *server, char *key)
{
	unsigned int hash = server->hash_function(key);
	int index = hash % server->hmax;
	ll_node_t *curr = server->buckets[index]->head;
	if (server->buckets[index]->size == 0) {
		//printf("blabla1\n\n");
		return NULL;
	}

	for (unsigned int i = 0; i < server->buckets[index]->size; i++) {
		//printf("blabla2\n\n");
		if (!server->compare_function(key, ((info *)curr->data)->key)) {
			return ((info *)curr->data)->value;
			//	printf("blabla3\n\n");
		}
		curr = curr->next;
	}

	return NULL;
}
// mi se da o cheie, trebuie sa o caut in server si i dau remove din lista cheii respective
void server_remove(server_memory *server, char *key)
{
	unsigned int hash = server->hash_function(key);
	int index = hash % server->hmax;
	ll_node_t *curr = server->buckets[index]->head;
	unsigned int i;
	if (server_retrieve(server, key)) {// exista nodul
		for (i = 0; i < server->buckets[index]->size; i++) {
			if (!server->compare_function(key, ((info *)curr->data)->key))
				break;
			curr = curr->next;
		}
		curr = ll_remove_nth_node(server->buckets[index], i);
		server->size--;
		key_val_free_function(curr->data);
		free(curr);
	}
}

//eliberez toata memoria: key din bucket, bucket si server
void free_server_memory(server_memory *server)
{
	for (unsigned int i = 0; i < server->hmax; i++)
		ll_free(&server->buckets[i]);
	free(server->buckets);
	free(server);
}
