/**
 *
 * Practica 4 de SO2, curs 2017-2018
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "red-black-tree.h"

#define MAXLINE      200
#define MAGIC_NUMBER 0x0133C8F9

/**
 *
 *  Counts the number of nodes of a tree
 *
 */

int count_nodes_recursive(Node *x)
{
  int nodes;

  nodes = 0;

  /* Analyze the children */
  if (x->right != NIL)
    nodes += count_nodes_recursive(x->right);

  if (x->left != NIL)
    nodes += count_nodes_recursive(x->left);

  /* Take into account the node itself */
  nodes += 1;

  return nodes;
}

int count_nodes(RBTree *tree)
{
  int nodes;

  nodes = count_nodes_recursive(tree->root);

  return nodes;
} 

/**
 *
 *  Insert word in a tree. This is the function is the same for both 
 *  local and global structures. Blocking functions are used outside
 *  this function. It may not be the optimum solution.
 *
 */

void insert_word_tree(RBTree *tree, char *paraula, int num_vegades)
{
  RBData *tree_data;
  char *paraula_copy;

  /* Search the work in the tree */
  tree_data = findNode(tree, paraula);

  if (tree_data != NULL) {
    //printf("Incremento comptador %s a l'arbre\n", paraula);

    /* We increment the number of times current item has appeared */
    tree_data->num_vegades += num_vegades;

  } else {
    //printf("Insereixo %s a l'arbre\n", paraula);

    /* If the key is not in the list, allocate memory for the data and
     * insert it in the list */

    paraula_copy = malloc(sizeof(char) * (strlen(paraula)+1));
    strcpy(paraula_copy, paraula);

    tree_data = malloc(sizeof(RBData));
    tree_data->key = paraula_copy;
    tree_data->num_vegades = num_vegades;

    insertNode(tree, tree_data);
  }
}

/**
 *
 *  Process each line that is received from pdftotext: extract the
 *  words that are contained in it and insert them in the tree.
 *
 */

void process_line(char *line, RBTree *tree)
{

  int i, j, is_word, len_word, len_line;
  char paraula[MAXLINE];

  i = 0;

  len_line = strlen(line);

  /* Search for the beginning of a candidate word */

  while ((i < len_line) && (isspace(line[i]) || (ispunct(line[i])))) i++; 

  /* This is the main loop that extracts all the words */

  while (i < len_line)
  {
    j = 0;
    is_word = 1;

    /* Extract the candidate word including digits if they are present */

    do {

      if (isalpha(line[i]))
        paraula[j] = line[i];
      else 
        is_word = 0;

      j++; i++;

      /* Check if we arrive to an end of word: space or punctuation character */

    } while ((i < len_line) && (!isspace(line[i])) && (!ispunct(line[i])));

    /* If word insert in list */

    if (is_word) {

      /* Put a '\0' (end-of-word) at the end of the string*/
      paraula[j] = 0;

      /* Now transform to lowercase */
      len_word = j;

      for(j = 0; j < len_word; j++)
        paraula[j] = tolower(paraula[j]);

      insert_word_tree(tree, paraula, 1); 

    } /* if is_word */

    /* Search for the beginning of a candidate word */

    while ((i < len_line) && (isspace(line[i]) || (ispunct(line[i])))) i++; 

  } /* while (i < len_line) */
}

/**
 *
 *  Copy local to global tree 
 *
 */

void tree_copy_local2global_recursive(Node *x, RBTree *tree_global)
{
  if (x->right != NIL)
    tree_copy_local2global_recursive(x->right, tree_global);

  if (x->left != NIL)
    tree_copy_local2global_recursive(x->left, tree_global);

  insert_word_tree(tree_global, x->data->key, x->data->num_vegades);
}


void tree_copy_local2global(RBTree *tree_file, RBTree *tree_global)
{
  tree_copy_local2global_recursive(tree_file->root, tree_global);
}

/**
 *
 *  Create the tree. 
 *
 */

RBTree *create_tree_files(int num_files, char **filename_texts)
{
  FILE *fp_file; 

  char line[MAXLINE], *filename;
  int i;

  RBTree *tree; 

  /* Allocate memory for tree */
  tree = (RBTree *) malloc(sizeof(RBTree));

  /* Initialize the tree */
  initTree(tree);

  /* Observe that finished is a local variable, not a global one */
  for(i = 0; i < num_files; i++)
  {
      /* Allocate  memory for local tree */

      RBTree *tree_file;
      tree_file = (RBTree *) malloc(sizeof(RBTree));

      /* Initialize the tree */
      initTree(tree_file);

      filename = filename_texts[i];
      printf("Processant fitxer %s\n", filename);

      /** This is the command we have to execute. Observe that we have to specify
       * the parameter "-" in order to indicate that we want to output result to
       * stdout.  In addition, observe that we need to specify \n at the end of the
       * command to execute. 
       */

      fp_file = fopen(filename, "r");
      if (!fp_file)
      {
          printf("ERROR: no puc crear canonada per al fitxer %s.\n", line);
          continue;
      }

      while (fgets(line, MAXLINE, fp_file) != NULL) {
          /* Remove the \n at the end of the line */

          line[strlen(line) - 1] = 0;

          /* Process the line */

          process_line(line, tree_file); 
      }

      fclose(fp_file);

      /* Copy all data from local tree to global tree */

      tree_copy_local2global(tree_file, tree);

      /* Delete local tree */

      deleteTree(tree_file);
      free(tree_file);
  }


  return tree;
}

/**
 *
 *  Create the tree. 
 *
 */

RBTree *create_tree(char *filename)
{
    FILE *fp;
    RBTree *tree;

    int i, num_files;
    char line[MAXLINE], **filename_texts;

    /* Open filename that contains all files */
    fp = fopen(filename, "r");
    if (!fp) {
        printf("ERROR: no he pogut obrir fitxer %s.\n", filename);
        return NULL;
    }

    /* Llegim el fitxer. Suposem que el fitxer esta en un format correcte */
    fgets(line, MAXLINE, fp);
    num_files = atoi(line);
    filename_texts = (char **) malloc(sizeof(char *) * num_files); 

    /* Llegim els noms dels fitxers a processar */
    for(i = 0; i < num_files; i++)
    {
        fgets(line, MAXLINE, fp); 
        line[strlen(line)-1]=0;

        /* Be sure we can acces the file */
        if(access(line, R_OK ) == -1) {
            printf("ERROR: no puc obrir fitxer d'entrada %s.\n", line);
            continue;
        }

        filename_texts[i] = (char *) malloc(sizeof(char) * (strlen(line) + 1));
        strcpy(filename_texts[i], line);
    }

    fclose(fp);

    /* Create tree from files */

    tree = create_tree_files(num_files, filename_texts); 

    /* Free dynamic memory */

    for(i = 0; i < num_files; i++)
        free(filename_texts[i]);

    free(filename_texts); 

    return tree;
}

/**
 *
 *  Save tree to disc
 *
 */

void save_tree_recursive(Node *x, FILE *fp)
{
    int i;

    if (x->right != NIL)
        save_tree_recursive(x->right, fp);

    if (x->left != NIL)
        save_tree_recursive(x->left, fp);

    i = strlen(x->data->key);
    fwrite(&i, sizeof(int), 1, fp);

    fwrite(x->data->key, sizeof(char), i, fp);

    i = x->data->num_vegades;
    fwrite(&i, sizeof(int), 1, fp);
}


void save_tree(RBTree *tree, char *filename)
{
    FILE *fp;

    int magic, nodes;

    fp = fopen(filename, "w");
    if (!fp) {
        printf("ERROR: could not open file for writing.\n");
        return;
    }

    magic = MAGIC_NUMBER;
    fwrite(&magic, sizeof(int), 1, fp);

    nodes = count_nodes(tree);
    fwrite(&nodes, sizeof(int), 1, fp);

    save_tree_recursive(tree->root, fp);

    fclose(fp);  
}

/**
 *
 *  Load tree from disc
 *
 */

RBTree *load_tree(char *filename)
{
    FILE *fp;
    RBTree *tree;
    RBData *tree_data;

    int i, magic, nodes, len, num_vegades;
    char *paraula;

    fp = fopen(filename, "r");
    if (!fp) {
        printf("ERROR: could not open file for reading.\n");
        return NULL;
    }

    /* Read magic number */
    fread(&magic, sizeof(int), 1, fp);
    if (magic != MAGIC_NUMBER) {
        printf("ERROR: magic number is not correct.\n");
        return NULL;
    }

    /* Read number of nodes */
    fread(&nodes, sizeof(int), 1, fp);
    if (nodes <= 0) {
        printf("ERROR: number of nodes is zero or negative.\n");
        return NULL;
    }

    /* Allocate memory for tree */
    tree = (RBTree *) malloc(sizeof(RBTree));

    /* Initialize the tree */
    initTree(tree);

    /* Read all nodes. If an error is produced, the current read tree is
     * returned to the user. */
    for(i = 0; i < nodes; i++)
    {
        fread(&len, sizeof(int), 1, fp);
        if (len <= 0) {
            printf("ERROR: len is zero or negative. Not all tree could be read.\n");
            return tree;
        }

        paraula = malloc(sizeof(char) * (len + 1));
        fread(paraula, sizeof(char), len, fp);
        paraula[len] = 0;

        fread(&num_vegades, sizeof(int), 1, fp);
        if (num_vegades <= 0) {
            printf("ERROR: num_vegades is zero or negative. Not all tree could be read.\n");
            free(paraula);
            return tree;
        }

        tree_data = malloc(sizeof(RBData));

        tree_data->key = paraula;
        tree_data->num_vegades = num_vegades;

        insertNode(tree, tree_data); 
    }

    fclose(fp);

    return tree;
}


/**
 * 
 *  Menu
 * 
 */

int menu() 
{
    char str[5];
    int opcio;

    printf("\n\nMenu\n\n");
    printf(" 1 - Creacio de l'arbre\n");
    printf(" 2 - Emmagatzemar arbre a disc\n");
    printf(" 3 - Llegir arbre de disc\n");
    printf(" 4 - Consultar informacio d'un node\n");
    printf(" 5 - Sortir\n\n");
    printf("   Escull opcio: ");

    fgets(str, 5, stdin);
    opcio = atoi(str); 

    return opcio;
}

/** 
 *
 * Search the number of times a word appears in the tree
 *
 */

void search_word(RBTree *tree, char *word)
{
    RBData *tree_data;

    tree_data = findNode(tree, word);

    if (tree_data)
        printf("La paraula %s ha aparegut %d vegades als documents analitzats\n", word, tree_data->num_vegades);
    else
        printf("La paraula %s no apareix a l'arbre\n", word);
}


/**
 * 
 *  Main procedure
 *
 */

int main(int argc, char **argv)
{
    char str[MAXLINE];
    int opcio;

    RBTree *tree;

    if (argc != 1)
        printf("Opcions de la linia de comandes ignorades\n");

    /* Inicialitzem a un punter NULL */
    tree = NULL;

    do {
        opcio = menu();
        printf("\n\n");

        switch (opcio) {
            case 1:
                if (tree) {
                    printf("Alliberant arbre actual\n");
                    deleteTree(tree);
                    free(tree);
                }

                printf("Introdueix fitxer que conte llistat fitxers: ");
                fgets(str, MAXLINE, stdin);
                str[strlen(str)-1]=0;

                printf("Creant arbre...\n");
                tree = create_tree(str);
                break;

            case 2:
                if (!tree) {
                    printf("No hi ha cap arbre a memoria\n");
                    break;
                }

                printf("Introdueix el nom de fitxer en el qual es desara l'arbre: ");
                fgets(str, MAXLINE, stdin);

                str[strlen(str)-1]=0;

                printf("Desant arbre...\n");
                save_tree(tree, str);
                break;

            case 3:
                if (tree) {
                    printf("Alliberant arbre actual\n");
                    deleteTree(tree);
                    free(tree);
                }

                printf("Introdueix nom del fitxer amb l'arbre: ");
                fgets(str, MAXLINE, stdin);

                str[strlen(str)-1]=0;

                printf("Llegint arbre de disc...\n");
                tree = load_tree(str);
                break;

            case 4:
                if (!tree) {
                    printf("No hi ha cap arbre a memoria\n");
                    break;
                }

                printf("Introdueix la paraula: ");
                fgets(str, MAXLINE, stdin);
                str[strlen(str)-1]=0;

                if (strlen(str) == 0) {
                    printf("No s'ha introduit cap paraula\n");
                    break;
                }

                search_word(tree, str);
                break;

            case 5:
                if (tree) {
                    printf("Alliberant memoria\n");
                    deleteTree(tree);
                    free(tree);
                }
                break;

            default:
                printf("Opcio no valida\n");

        } /* switch */
    }
    while (opcio != 5);

    return 0;
}



