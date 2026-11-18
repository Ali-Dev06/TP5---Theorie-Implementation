/* Analyseur syntaxique LL(1) par table et pile.
   Algorithme du cours (slide 31) : table M du slide 30. */
#include <stdio.h>
#include <string.h>

#define MAX_MOT 100
#define MAX_PILE 200

/* Dans le programme, Q represente R', U represente T' et H represente F'. */
const char nonTerminaux[] = "RQTUFHP";
const char terminaux[] = "ab()+*$";

/* Les productions sont numerotees de 1 a 12.
   Une chaine vide signifie epsilon. */
const char *productions[] = {
    "",       /* 0 : aucune production */
    "TQ",     /* 1 : R  -> T R' */
    "+TQ",    /* 2 : R' -> + T R' */
    "",       /* 3 : R' -> epsilon */
    "FU",     /* 4 : T  -> F T' */
    "FU",     /* 5 : T' -> F T' */
    "",       /* 6 : T' -> epsilon */
    "PH",     /* 7 : F  -> P F' */
    "*H",     /* 8 : F' -> * F' */
    "",       /* 9 : F' -> epsilon */
    "(R)",    /* 10 : P  -> ( R ) */
    "a",      /* 11 : P  -> a */
    "b"       /* 12 : P  -> b */
};

/* SUIVANT de chaque non-terminal, dans l'ordre R, R', T, T', F, F', P (mode panique) */
const char *suivant[] = { ")$", ")$", "+)$", "+)$", "(ab+)$", "(ab+)$", "*(ab+)$" };

/* Table M : lignes R, R', T, T', F, F', P
              colonnes a, b, (, ), +, *, $ */
const int table[7][7] = {
    { 1,  1,  1,  0,  0,  0,  0 }, /* R  */
    { 0,  0,  0,  3,  2,  0,  3 }, /* R' */
    { 4,  4,  4,  0,  0,  0,  0 }, /* T  */
    { 5,  5,  5,  6,  6,  0,  6 }, /* T' */
    { 7,  7,  7,  0,  0,  0,  0 }, /* F  */
    { 9,  9,  9,  9,  9,  8,  9 }, /* F' */
    {11, 12, 10,  0,  0,  0,  0 }  /* P  */
};

int analyser(const char mot[])   /* retourne le nombre d'erreurs */
{
    int erreurs = 0;
    char entree[MAX_MOT + 2];
    char pile[MAX_PILE];
    int position = 0;
    int sommet = 0;

    /* Le symbole $ marque la fin de l'entree. */
    strcpy(entree, mot);
    strcat(entree, "$");

    /* Initialisation de la pile : $ puis le symbole de depart R. */
    pile[sommet++] = '$';
    pile[sommet++] = 'R';

    while (1) {
        char X = pile[sommet - 1]; /* sommet de la pile */
        char a = entree[position]; /* prochain caractere de l'entree */

        /* Cas 1 : X est un non-terminal, on choisit une production dans M. */
        if (strchr(nonTerminaux, X) != NULL) {
            int ligne = (int)(strchr(nonTerminaux, X) - nonTerminaux);
            const char *caseTable = strchr(terminaux, a);
            int numeroProduction;
            const char *partieDroite;

            numeroProduction = (caseTable == NULL) ? 0
                             : table[ligne][(int)(caseTable - terminaux)];
            if (numeroProduction == 0) {
                /* MODE PANIQUE : on saute les lettres jusqu'a un terminal de SUIVANT(X)
                   (il contient toujours $, donc la boucle s'arrete), puis on abandonne X. */
                erreurs++;
                printf("Erreur : %c inattendu (position %d)\n", a, position + 1);
                while (strchr(suivant[ligne], entree[position]) == NULL)
                    position++;
                sommet--;
                continue;
            }

            /* Emission de la production utilisee (comme dans le cours). */
            printf("Production %d\n", numeroProduction);

            sommet--; /* retirer le non-terminal de la pile */
            partieDroite = productions[numeroProduction];

            /* Empiler la production de droite a gauche.
               Si elle vaut epsilon, la boucle ne pousse aucun symbole. */
            for (int i = (int)strlen(partieDroite) - 1; i >= 0; i--) {
                if (sommet >= MAX_PILE)
                    return erreurs + 1;
                pile[sommet++] = partieDroite[i];
            }
        }
        /* Cas 2 : X = $. Le mot est accepte seulement si l'entree est aussi finie. */
        else if (X == '$') {
            if (a != '$') {                     /* il reste des lettres apres la fin du mot */
                erreurs++;
                printf("Erreur : %c en trop (position %d)\n", a, position + 1);
            }
            return erreurs;
        }
        /* Cas 3 : X est un terminal identique au caractere lu :
           on le retire de la pile et on avance dans l'entree. */
        else if (X == a) {
            sommet--;
            position++;
        }
        /* Sinon : terminal attendu different du caractere lu. */
        else {
            erreurs++;                          /* terminal attendu absent : on le retire */
            printf("Erreur : %c attendu a la position %d\n", X, position + 1);
            sommet--;
        }
    }
}

int main(void)
{
    char mot[MAX_MOT];
    char ligne[MAX_MOT + 2];
    size_t longueur;

    printf("Entrez un mot : ");
    if (fgets(ligne, sizeof ligne, stdin) == NULL) {
        printf("Erreur de lecture.\n");
        return 1;
    }

    /* Refuser une ligne trop longue au lieu d'analyser seulement son debut. */
    if (strchr(ligne, '\n') == NULL) {
        int caractere = getchar();
        if (caractere != EOF) {
            while (caractere != '\n' && caractere != EOF)
                caractere = getchar();
            printf("Erreur : mot trop long (maximum 99 caracteres).\n");
            return 1;
        }
    }

    longueur = strcspn(ligne, "\r\n");
    if (longueur == 0 || longueur >= sizeof mot) {
        printf("Erreur : entrez un mot de 1 a 99 caracteres.\n");
        return 1;
    }
    memcpy(mot, ligne, longueur);
    mot[longueur] = '\0';

    /* Le dollar est ajoute par le programme, il ne fait pas partie du mot. */
    if (strchr(mot, '$') != NULL) {
        printf("Refuse : '$' est reserve pour marquer la fin du mot.\n");
        return 0;
    }

    int erreurs = analyser(mot);
    if (erreurs == 0)
        printf("Mot accepte.\n");
    else
        printf("Mot refuse (%d erreur(s)).\n", erreurs);

    return 0;
}