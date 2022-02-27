/* a simple algorithm for stemminng and POS tagging using a dictionary */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* constants */
#define MAX_WORD 100   // max number of unique words in a dictionary
#define START '$'   // indicates start of word
#define MAX_CHAR 22    // max number of characters per word
#define MAX_POS 5   // max number of POS tags per word
#define MAX_POS_CHAR 4  // max number of letters per POS tag
#define END '#' // ending line of a word in the dictionary
#define END_DICT '*' // end of dictionary
#define MAX_FORM 4   // max number of forms per word 
#define MAX_VAR_CHAR 25   // max number of letters per variation
#define MAX_SEN_CHAR 25   // max number of letters per word in a sentence

#define PAT '0'  // past tense form
#define PAP '1'  // past participle form
#define PEP '2' // present participle form
#define PLR '3' // plural form

#define NOT_FOUND 0 // key is not found in dict
#define FOUND 1 // key is found in dict

#define DELIM "=========================="
#define STG1 "Stage 1"
#define STG2 "Stage 2"
#define STG3 "Stage 3"
#define STG4 "Stage 4"

/* typedefs */
typedef struct {
    char word[MAX_CHAR+1];    // the word itself
    char POS_tags[MAX_POS][MAX_POS_CHAR+1]; // the POS tags
    int ntags;  // number of POS tags
    char vars[MAX_FORM*MAX_VAR_CHAR+1];    // the variations forms
    int nvars;  // number of variation forms
} word_t;

typedef struct node node_t;

struct node {
    char word[MAX_SEN_CHAR+1];
    node_t* next;
};

typedef struct {
    node_t* head;
    node_t* foot;
} sent_t;

/* function prototypes */
void read_word(word_t dict[MAX_WORD], int* nwords);
void print_stg1(word_t dict[MAX_WORD], int* nwords);
void print_stg2(word_t dict[MAX_WORD], int nwords);
int num_vars(char vars[], int nwords);
sent_t* new_sent(void);
node_t* new_node(void);
sent_t* read_sentence(sent_t* sent);
sent_t* insert_at_foot(sent_t* sent);
void print_stg3(sent_t* sent, word_t dict[], int nwords);
int bisearch(word_t dict[], int lo, int hi, char* key, int* locn);
void print_stg4(sent_t* sent, word_t dict[], int nwords);
int ssearch(char* word, char* vars, int wlen, int vlen);
void free_list(sent_t* sent);

/* the program */
int main(int argc, char* argv[]) {
    word_t dict[MAX_WORD];  // the dictionary
    int nwords;
    sent_t* sent;
    print_stg1(dict, &nwords);  // stage 1
    print_stg2(dict, nwords);   // stage 2
    sent = new_sent();  // create new list
    sent = read_sentence(sent); // read in the sentence
    print_stg3(sent, dict, nwords);   // stage 3
    print_stg4(sent, dict, nwords);   // stage 4
    free_list(sent);
}

/* reads the dictionary and store the words in an array of structs */
void read_word(word_t dict[MAX_WORD], int* nwords) {
    /* wc - word count, cc - character count */
    int c, line1=0, wc=0, cc=0, line2=0, line3=0;
    while ((c=getchar())!=END_DICT) {   // ignore the sentence
        if (c==START) { // new word detected
            wc++;
            dict[wc-1].ntags = dict[wc-1].nvars = line3 = cc = 0;
            line1 = 1;
        } else if (c=='\n') {   
            if (line1) {    // end of line 1 and prepare for line 2 input 
                line2 = 1;
                dict[wc-1].word[cc] = '\0';
                line1 = cc = 0;
            } else if (line3) { // end of line 3 and preprare for line 1 input 
                dict[wc-1].vars[cc] = '\0';  
            }
        } else if (c==' ' || c==END) {  // end of a POS tag reached
            dict[wc-1].POS_tags[dict[wc-1].ntags][cc] = '\0';
            dict[wc-1].ntags++;
            cc = 0;
            line2 = (c!=END);   // end of line 2 and prepare for line 3 input
            line3 = (c==END);
        } else if (line1) { // need to store dicitonary word
            dict[wc-1].word[cc]=c;
            cc++;
        } else if (line2) { // need to store POS tags
            dict[wc-1].POS_tags[dict[wc-1].ntags][cc++] = c;
        } else if (line3) { // need to store variation forms 
            dict[wc-1].vars[cc++]=c;
        } 
    }
    *nwords = wc;
}

/* print one dictionary word */
void print_stg1(word_t dict[MAX_WORD], int* nwords) {
    int i;
    printf("%s%s%s\n", DELIM, STG1, DELIM); // the heading
    read_word(dict, nwords);    // read all the dictionary words
    printf("Word 0: %s\n", dict[0].word);
    printf("POS: ");
    /* print information on the first dictionary word */
    for (i=0; i<dict[0].ntags; i++) {   
        printf("%s ", dict[0].POS_tags[i]);
    }
    printf("\nForm: %s\n", dict[0].vars);
}

/* print the total number of words in the dictionary and the avg number of 
variation forms per word */
void print_stg2(word_t dict[MAX_WORD], int nwords) {
    int totv=0, i;
    printf("%s%s%s\n", DELIM, STG2, DELIM); // the heading
    printf("Number of words: %d\n", nwords);    
    /* count the number of variations for each word, and compute the total 
    number of variations */
    for (i=0; i<nwords; i++) {
        dict[i].nvars = num_vars(dict[i].vars, nwords); // stored in struct
        totv += dict[i].nvars;  
    }
    printf("Average number of variation forms per word: %.2f\n", 
    1.0*totv/(nwords));
}

/* compute the number of variations */
int num_vars(char vars[], int nwords) {
    int nvars=0;
    while (*vars) { 
        /* new variation detected */
        if (*vars==PAT || *vars==PAP || *vars==PEP || *vars==PLR) {
            nvars++;
        }
        vars += 1;
    }
    return nvars;
}

/* returns a new list (sent_t) with one node built in */
sent_t* new_sent(void) {
    sent_t* sent;
    node_t* node;
    sent = (sent_t*)malloc(sizeof(*sent));  // the new sentence
    node = new_node();  // initialised with one new node
    sent->head = sent->foot = node;
    return sent;
}

/* returns a new node */
node_t* new_node(void) {
    node_t* new;
    new = (node_t*)malloc(sizeof(*new));
    new->next = NULL;
    return new;
}

/* read in the sentence and stores the words in a linked list */
sent_t* read_sentence(sent_t* sent) {
    int c, cc=0;
    node_t* curr=sent->head;
    while ((c=getchar())!=EOF) {
        if (c!=END_DICT && isalpha(c)) {    // record the letters of the word
            curr->word[cc++] = c;
        } else if (c!=END_DICT && c==' ') { // new word detected
            curr->word[cc] = '\0';
            // add a new node in order to store the next word
            insert_at_foot(sent);   
            curr = curr->next;
            cc = 0;
        } 
    } 
    curr->word[cc] = '\0';
    return sent;
}

/* add a new node at the foot of the linked list */
sent_t* insert_at_foot(sent_t* sent) {
    node_t* new=new_node();
    sent->foot->next = new;
    sent->foot = new;
    return sent;
}

// print stage 3 output: each word in the sentence and the associated POS tags 
void print_stg3(sent_t* sent, word_t dict[], int nwords) {
    node_t* curr;
    int locn, i;
    printf("%s%s%s\n", DELIM, STG3, DELIM); // the heading;
    curr = sent->head;
    while (curr) {  // going through the list
        printf("%-26s", curr->word);    // the word 
        if (bisearch(dict, 0, nwords, curr->word, &locn)) {// word found in dict
            for (i=0; i<dict[locn].ntags; i++) {
                printf("%s ", dict[locn].POS_tags[i]);
            }
        } else {    // word not found in dict
            printf("NOT_FOUND");
        }
        printf("\n");
        curr = curr->next;
    }
}

/* binary search for looking up each word in the from the sentence in the 
   dictionary */
int bisearch(word_t dict[], int lo, int hi, char* key, int* locn) {
    int mid, outcome;
    if (lo>=hi) {  // if key is in dict, then it's between dict[lo] & dict[hi-1]
        return NOT_FOUND;
    }
    mid = (lo+hi)/2;
    /* key smaller than mid, must be in the left half of the dict */
    if ((outcome=strcmp(key, (dict+mid)->word))<0) {    
        return bisearch(dict, lo, mid, key, locn);
    /* key bigger than mid, must be in the right half of the dict */
    } else if (outcome > 0) {   
        return bisearch(dict, mid+1, hi, key, locn);
    } else {    // key cannot be found in the dict
        *locn = mid;
        return FOUND;
    }
}

/* print stage 4 output: for each word in sentence, ouput the word itself, its 
root form and its POS tags */
void print_stg4(sent_t* sent, word_t dict[], int nwords) {
    node_t* curr;
    int i, j, wlen, vlen, found=1;
    printf("%s%s%s\n", DELIM, STG4, DELIM); // the heading;
    curr = sent->head;
    while (curr) {  // going through the list
        printf("%-26s", curr->word);    // the word 
        for (i=0; i<nwords; i++) {  // find and print the root form
            wlen = strlen(curr->word);  // length of word in sentence
            vlen = strlen(dict[i].vars);    // length of the string of vars
            /* root form of the word is found  */
            if ((found=ssearch(curr->word, dict[i].vars, wlen, vlen))) {
                printf("%-26s", dict[i].word);
                break;
            /* word already in root form */
            } else if ((found=!strcmp(curr->word, dict[i].word))) {
                printf("%-26s", dict[i].word);
                break;
            }
        }
        if (found) {    // print POS tags for word in dictionary
            for (j=0; j<dict[i].ntags; j++) {
                printf("%s ", dict[i].POS_tags[j]);
            }
        } else {    // print root form and POS tags for word not in dictionary 
            printf("%-26s", curr->word);
            printf("NOT FOUND");
        }
        printf("\n");
        curr = curr->next;
    }
}

/* sequential pattern search algorithm */
int ssearch(char* word, char* vars, int wlen, int vlen) {
    int s=0, i; // s = position of 1st char of the pattern in the text 
    while (s<=vlen-wlen) {
        for (i=0; i<wlen; i++) {
            if (vars[s+i]!=word[i]) {   // mismtach detected
                break;
            }
        }
        if (i==wlen) {  // a match is found 
            return FOUND;
        }
        s++;
    }
    return NOT_FOUND;
}

/* free the memory allocated to the list */
void free_list(sent_t* sent) {
    node_t* curr=sent->head, *prev;
    while (curr) {  // traverse through the list 
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(sent);
}


