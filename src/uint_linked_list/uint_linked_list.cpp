#include <stdexcept>
#include <string>
#include <bitset>
#include "uint_linked_list.h"
#include "../utils/common_utils.h"

uint_linked_list::uint_linked_list() : head(nullptr) {}

uint_linked_list::uint_linked_list(const uint_linked_list& other) {
    head = nullptr;

    Node* cur = other.head;
    while (cur != nullptr) {
        append(cur->data);
        cur = cur->next;
    }
}

uint_linked_list::~uint_linked_list() {
    while (head != nullptr) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
    head = nullptr;
}

void uint_linked_list::append(unsigned int data) {
    Node* newNode = new Node{ data, nullptr };
    if (head == nullptr) {
        head = newNode;
    }
    else {
        Node* current = head;
        while (current->next != nullptr) {
            current = current->next;
        }
        current->next = newNode;
    }
}

// insert before the index, new elem will become new elem on index
void uint_linked_list::insert(int index, unsigned int data) {
    if (index < 0) return;
    Node* newNode = new Node{ data, nullptr };
    if (index == 0) {
        newNode->next = head;
        head = newNode;
    }
    else {
        Node* current = head;
        for (int i = 0; i < index - 1 && current != nullptr; i++) {
            current = current->next;
        }
        if (current != nullptr) {
            newNode->next = current->next;
            current->next = newNode;
        }
    }
}

void uint_linked_list::remove(unsigned int data) {
    if (head == nullptr) return;
    if (head->data == data) {
        Node* temp = head;
        head = head->next;
        delete temp;
        return;
    }
    Node* current = head;
    while (current->next != nullptr && current->next->data != data) {
        current = current->next;
    }
    if (current->next != nullptr) {
        Node* temp = current->next;
        current->next = current->next->next;
        delete temp;
    }
}

int uint_linked_list::length() {
    int count = 0;
    Node* current = head;
    while (current != nullptr) {
        count++;
        current = current->next;
    }
    return count;
}

std::string uint_linked_list::to_bit_string()
{
    std::string res = "";
    
    Node* current = head;
    while (current != nullptr) {
        res = res + std::bitset<INF_INT_DIGIT_SIZE>(current->data).to_string();
        current = current->next;
    }
    return res;
}

unsigned int& uint_linked_list::operator[](int index) {
    Node* current = head;
    for (int i = 0; i < index && current != nullptr; i++) {
        current = current->next;
    }
    if (current != nullptr) {
        return current->data;
    }
    else {
        throw std::out_of_range("Index out of range");
    }
}

uint_linked_list& uint_linked_list::operator=(const uint_linked_list& other) {
    if (this != &other) { 
        uint_linked_list *temp = new uint_linked_list(other);
        head = temp->head;
    }

    return *this;
}
