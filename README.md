
# Table of Contents

1.  [Instructions](#org71e166d)
2.  [Messagerie](#org2b1fe7a)
    1.  [Squelette de code](#orgc93c7e1)
3.  [Première étape : serveur multi-processus](#org715efd6)
    1.  [Évaluation](#orgcd3ce89)
4.  [Deuxième étape : serveur multi-thread](#org95abdaa)
    1.  [Suggestions](#orgd955449)
    2.  [Évaluation](#orgaaaa4c7)
5.  [Troisième étape : commandes](#orgc8be709)
    1.  [Suggestions](#orgfe82937)
        1.  [Structure multi-thread (optionnelle)](#org3cfd075)
    2.  [Évaluation](#org3ce118e)



<a id="org71e166d"></a>

# Instructions

Le but de ce TP est d'apprendre la programmation client/serveur
multi-processus et multi-threaded. À partir d'un squelette de code
d'une application client/serveur minimale, vous devez implanter une
application de messagerie (chat) entre plusieurs clients.

Ce TP est à rendre et il sera noté. Cette note comptera pour 40% de
votre note finale. L'échéance finale pour rendre le code de votre
programme est le **vendredi 31 mars 2023**.

Il est **fortement recommandé** de travailler en binôme (sauf cas
exceptionnels). Les deux étudiants d'un binôme devront coder. Nous
allons vérifier sur votre dépôt gitlab les messages de *commit* des
deux étudiants. Si un seul de deux aura fait des *commit*, nous
allons donner une note de zéro **aux deux étudiants**.

Si vous voulez travailler seul, vous devez contacter votre intervenant
et justifier votre demande avec des raisons spécifiques.

Une soutenance est prévue la semaine du 3 avril. Vous allez passer
individuellement devant votre intervenant qui vous posera des
questions pour vérifier que vous avez bien compris le fonctionnement
de votre code. Chaque réponse du genre "*Je ne sais pas, c'est mon
binôme qui a codé cette partie*" sera pénalisé avec une note négative.


<a id="org2b1fe7a"></a>

# Messagerie


<a id="orgc93c7e1"></a>

## Squelette de code

Le code dans le répertoire consiste des fichiers suivants :

-   <mtc_client.c> : le code du programme client ;
-   <mtc_server.c> : le code du programme serveur ;
-   <console.c> et <console.h> : le code pour l'interface terminal du client ;
-   <protocol.h> : la spécification des messages échangés entre clients et serveur ;
-   <utils.h> : des fonctions et macros d'utilité.
-   <Makefile> : le makefile

Pour compiler les deux programmes, il suffit de lancer `make`.
Pour exécuter le serveur, saisissez dans un terminal la commande

    ./mtc_server <port_number>

ou `port_number` est un numéro de port, par exemple `8008`.
Si vous voulez un log de tous les messages du serveur, ajoutez l'option `-v` (*verbose*) :

    ./mtc_server -v <port_number>

Pour lancer le client, dans un autre terminal saisissez la commande suivante :

    ./mtc_client <host_name> <port_number> <pseudo>

ou `host_name` est l'adresse IP4 du serveur. Si vous vous connectez
sur le même ordinateur du serveur, il suffit de spécifier
`localhost`. Le `port_number` doit être le même du serveur. Le
`pseudo` est le nom d'affichage de ce client.

Si tout marche correctement, vous verrez sur l'écran l'image suivante :

![img](mtc_client.png)

Vous pouvez saisir un message. Après "enter", le message est envoyé au
serveur, qui renvoie le même message en majuscules. En autres termes
le serveur fait un *echo* des messages du client. Pour sortir, il fait
saisir le message `[quit]` ou taper `CTRL-C` sur le clavier.

Si vous essayez de vous connecter avec un deuxième client, ce dernier
restera bloqué jusqu'à quand le premier client aura terminé.


<a id="org715efd6"></a>

# Première étape : serveur multi-processus

Dans une première phase, on permettra au serveur de gérer plusieurs
clients en même temps. Ce n'est pas encore un serveur de messagerie :
chaque client verra seulement ses propres messages.

Pour faire ça, chaque fois qu'un client sera connecté, le serveur
créera un processus fils qui traitera l'interaction avec ce client.

Attention : il faudra éviter les processus *zombie* ! Chaque fois qu'un
client termine, le processus fils correspondant doit terminer et le
processus père doit faire la `wait()` correspondant.

Cependant, il y a un problème : si le processus père est bloqué sur
l'`accept()`, il n'aura pas la possibilité de faire la `wait()`. Pour
résoudre ce problème, vous pouvez utiliser le signal `SIGCHLD` : si
vous installez un gestionnaire de signaux pour le signal `SIGCHLD`, à
chaque fois qu'un fils se termine, un signal `SIGCHLD` est envoyé au
père.


<a id="orgcd3ce89"></a>

## Évaluation

L'exercice est résolu si vous êtes capables de démontrer (avec une démo)
que :

-   plusieurs clients sont connectés en même temps au serveur ;
-   il n'y a jamais des processus zombies crée par le serveur.


<a id="org95abdaa"></a>

# Deuxième étape : serveur multi-thread

**ATTENTION** : Pour implémenter cette deuxième étape **créez un nouvelle
branche sur vote dépôt gitlab**, nommé "multithread", et codé sur cette
branche.

Il n'est pas facile d'implanter un programme de messagerie avec un
serveur multi-processus. En effet, un processus fils n'a pas accès
aux *sockets* utilisé par les autres processus et il ne peut pas
directement écrire aux autres clients pour distribuer les messages.

Pour réaliser une messagerie, on change de stratégie et on utilise les
*threads*.

Chaque client est géré par un thread qui attend un message du client
sur son propre *socket*. De que le message est arrivé, le thread
l'envoie aux autres clients en écrivant sur leurs *sockets* précédé
par son pseudo. Par exemple, si le client avec pseudo `A` envoie le
message `toto`, le message

    <A> : toto

est envoyé à tous les clients, y compris le client `A`.


<a id="orgd955449"></a>

## Suggestions

Il est donc nécessaire de préparer des structures globales où
mémoriser les clients connectés avec leur *pseudo* et leur *socket*.

-   Préparez une structure ou mémoriser les informations concernant un
    client. Par exemple :
    
        struct client {
                char pseudo[PSEUDO_SIZE];
                int socket;
                // autres ...
        };
-   Les clients actifs seront mémorisés dans un tableau global,
    accessible à tous les threads. Par exemple :
    
        struct client all_clients[MAX_CLIENTS];
-   Quand un nouveau client est connecté, le programme sélectionne une
    structure non encore utilisée dans le tableau, et il la remplie avec
    les données nécessaires. Le thread pour ce client se met en attente
    de nouveau messages sur le *socket* correspondant.
-   Quand un message arrive, le thread distribue le message à tous les
    autres clients en utilisant le tableau partagé.

**Attention** : comme le tableau est partagé, il faut le protéger avec des
*mutex* !

Vous devez éviter d'utiliser des appels à `malloc`. Toutes les
structures de données doivent être créés statiquement.


<a id="orgaaaa4c7"></a>

## Évaluation

Vous devez nous montrer une démo où plusieurs clients se connectent au
serveur et échangent des messages.

-   Il ne faut pas utiliser `malloc()` et `free()`
-   Le nombre maximum de clients connecté en même temps doit être limité
    à `4`. Montrez que si un 5ème client essaye de se connecter, la
    connexion reste en attente, ou elle est refusée.

Nous allons évaluer l'utilisation correcte des *mutex* sur votre
code. Toutes les structures de données partagées doivent être
protégées correctement.


<a id="orgc8be709"></a>

# Troisième étape : commandes

**ATTENTION** : Pour implémenter cette troisième étape **créez un
nouvelle branche sur vote dépôt gitlab**, nommé "commandes", et codé
sur cette branche.

Nous allons donner la possibilité aux clients d'envoyer des commandes
aux serveurs. Les commandes sont des chaînes de caractères entre
`[]`.

-   Commande `[stats]` : le serveur renvoie les informations suivantes
    -   date et heure de démarrage ;
    -   nombre de messages échanges ;
    -   nombre d'utilisateurs connectés.
-   Commande `[users]` : le serveur renvoie la liste des utilisateurs
    connectés.
-   Commande `[private] user` : le client passe en mode *privé* avec
    l'utilisateur `user` ; dorénavant, il recevra seulement les messages
    de `user` et il enverra ses messages qu'à `user`. Rien ne change
    pour l'utilisateur `user`. Si `user` n'est pas connecté, le serveur
    renvoie un message d'erreur.
-   Commande `[public]` : le client retourne en mode publique ;
    dorénavant, il recevra les messages de tous les utilisateurs.
-   Commande `[groupe] n` : dorénavant le client recevra les messages en
    groupe de `n` à la fois ; si d'autres clients lui envoie des
    messages (privées ou publiques), ils ne sont pas immédiatement
    envoyés, mais seulement après que `n` messages se sont cumulés. Par
    exemple, supposons qu'il y a trois clients, `A`, `B` et `C`
    connectés au serveur.
    
    1.  l'utilisateur `A` donne la commande `[groupe] 2` ;
    2.  l'utilisateur `B` envoie un message `M1` à tous ; le message est
        envoyé immédiatement à `C` qui reçoit `<B> : M1`, mais pas à `A`
        ;
    3.  l'utilisateur `C` envoie le message `M2` à tous ; le message est
        envoyé immédiatement à `B` qui reçoit `<C> : M2` ; `A` reçoit
        deux messages, `<B> : M1` et `<C> : M2`.
    
    La valeur maximum de `n` est `4` ; si un client spécifie `[groupe]
      n` avec n>4 ou n<=0, sa commande est ignorée et le serveur réponds
    avec un message d'erreur.
    
    La commande `[groupe] n` est orthogonale à la commande `[private]
      user`, les deux sont possibles en même temps. Pour revenir à la
    situation normale, l'utilisateur peut donner la commande `[groupe]
      1`.


<a id="orgfe82937"></a>

## Suggestions

Il faut faire évoluer la structure de données `struct client` pour
contenir plus d'informations. Aussi, d'autres variables globales
sont nécessaires pour les statistiques.

Il est toujours défendu d'utiliser `malloc()` et `free()`.


<a id="org3cfd075"></a>

### Structure multi-thread (optionnelle)

Si vous le souhaitez, il est possible (mais pas obligatoire) de
changer la structure du programme pour utiliser plus d'un thread
per client. Par exemple, pour chaque client il est possible de
créer deux threads : un thread qui fait les lectures du socket et
un thread pour faire les écritures ; vous utiliserez une structure
type producteurs/consommateur avec des *boîtes à message* pour
échanger entre clients. Par exemple, avec 2 clients, vous aurez la
structure suivante :

![img](prod-cons.png)

Dans la figure, `Prod1` et `Cons1` sont les threads qui gèrent le
client `Client 1`, et `Cons2` et `Prod2` gèrent le client `Client
   2`. Les boîtes `Boite1` et `Boîte2` servent à mémoriser les messages à
envoyer aux `Client1` et `Client2`, respectivement. Le thread
consommateur est normalement bloqué dans l'attente d'un message sur
la boîte, pendant que le thread producteur est bloqué dans
l'attente d'un message sur le socket.


<a id="org3ce118e"></a>

## Évaluation

Faire une démo pour montrer le fonctionnement de toutes les commandes
et de leurs combinaisons.

Nous allons évaluer la structure de votre code, l'utilisation des
*mutex* et, si nécessaire, des variables conditions. Vous aurez des
points supplémentaires si vous implémentez la structure
producteurs/consommateur.

