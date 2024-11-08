/* Copyright 2023 <Dragan Dragos Ovidiu> */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "/home/student/Desktop/de_predat/SDA/Tema 2/Load-Balancer/server.h"
#include "/home/student/Desktop/de_predat/SDA/Tema 2/Load-Balancer/utils.h"

#define KEY_LENGTH 128
#define VALUE_LENGTH 65536

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
	DIE(!list, "Nu s-a putut aloca");
	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;

	return list;
}

ll_node_t *ll_remove_nth_node(linked_list_t *list, unsigned int n)
{
	ll_node_t *prev, *curr;

	if (!list || !list->head) {
		return NULL;
	}

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
	DIE(!new_node, "Nu s-a putut aloca");
	new_node->data = malloc(list->data_size);
	DIE(!new_data, "Nu s-a putut aloca");
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
			key_val_free_function(curr->data);  // eliberez memoria din noduri
			free(curr);  // eliberez nodul
			curr = aux;
		}
	}
	free(*pp_list);  // sterg lista
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
	DIE(!server, "Nu s-a putut aloca");
	server->hmax = 9999;
	server->size = 0;
	server->hash_function = hash_function_k;
	server->key_val_free_function = key_val_free_function;
	server->compare_function = compare_function_strings;
	server->buckets = malloc(server->hmax * sizeof(linked_list_t *));
	DIE(!server->buckets, "Nu s-a putut aloca");
	for (unsigned int i = 0; i < server->hmax; i++)
		server->buckets[i] = ll_create(sizeof(info));

	return server;
}

// o sa primesc memoria unui server (ex: key, value)
void server_store(server_memory *server, char *key, char *value)
{
	unsigned int hash = server->hash_function(key);
	int index = hash % server->hmax;
	ll_node_t *curr = server->buckets[index]->head;

	if (server_retrieve(server, key)) {  // daca exista cheia
		for (unsigned int i = 0; i < server->buckets[index]->size; i++) {
			if (!server->compare_function(key, ((info *)curr->data)->key)) {
				break;
			}
			curr = curr->next;
		}
		memcpy(((info *)curr->data)->value, value, VALUE_LENGTH);
	} else {
		info *cop = malloc(sizeof(info));
		DIE(!cop, "Nu s-a putut aloca");

		cop->key = malloc(KEY_LENGTH);
		DIE(!cop->key, "Nu s-a putut aloca");

		cop->value = malloc(VALUE_LENGTH);
		DIE(!cop->value, "Nu s-a putut aloca");

		memcpy(cop->key, key, KEY_LENGTH);
		memcpy(cop->value, value, VALUE_LENGTH);
		ll_add_nth_node(server->buckets[index], server->buckets[index]->size, cop);
		server->size++;
		free(cop);
	}
}

char *server_retrieve(server_memory *server, char *key)
{
	unsigned int hash = server->hash_function(key);
	int index = hash % server->hmax;
	ll_node_t *curr = server->buckets[index]->head;
	if (server->buckets[index]->size == 0) {
		return NULL;
	}

	for (unsigned int i = 0; i < server->buckets[index]->size; i++) {
		if (!server->compare_function(key, ((info *)curr->data)->key)) {
			return ((info *)curr->data)->value;
		}
		curr = curr->next;
	}

	return NULL;
}

void server_remove(server_memory *server, char *key)
{
	unsigned int hash = server->hash_function(key);
	int index = hash % server->hmax;
	ll_node_t *curr = server->buckets[index]->head;
	unsigned int i;
	if (server_retrieve(server, key)) {  // exista nodul
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

// eliberez toata memoria: key din bucket, bucket si server
void free_server_memory(server_memory *server)
{
	for (unsigned int i = 0; i < server->hmax; i++)
		ll_free(&server->buckets[i]);
	free(server->buckets);
	free(server);
}
