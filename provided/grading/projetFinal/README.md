# README

## Avancement

Ce projet de C est entièrement fini et contient le rapport de nos différentes expériences.

## Exécutables supplémentaires

Ces expériences ont été menées grâce à deux utilitaires:

```
test-servers [-n N] [--] <number-of-tests>
test-parser
```

Le premier permet d'envoyer *number-of-tests* requêtes put à N serveurs par W différent.
W est incrémenté jusqu'à ce que le quorum n'est plus du tout atteint 3 fois de suite sur *number-of-tests* fois (e.g., N=10, W=5,6,7 échouent, donc le test s'arrête).
Pour qu'un W échoue, il faut **qu'aucun** des *number-of-tests* ne passe.

Le second permet de parser les données temporelles ainsi:

> SUCCÈS

> W, #succès, moyenne temps en ms, écart type temps en ms

> ÉCHECS

> W, #échecs, moyenne temps en ms, écart type temps en ms

## Temps

Pour ce projet nous avons passé plus de temps qu'il aurait fallu pour un cours de 2 crédits.
Mais cela n'est de loin pas du temps qui a été gaspillé, bien au contraire : ce projet nous a permis d'apprendre à coder en C et de comprendre les subtilités du langage, ainsi que ses défaults.

Heures de travail individuel estimées: 4 à 5 heures par semaine par personne.

Les premières semaines étaient certes plus simples et plus courtes mais comme elles étaient notées, elles prenaient beaucoup de temps de relecture afin d'assurer une bonne note.

