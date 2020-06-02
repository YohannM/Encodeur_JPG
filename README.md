# Encodeur_JPG

[![Build Status](https://travis-ci.org/joemccann/dillinger.svg?branch=master)](https://travis-ci.org/joemccann/dillinger)

Cet executable ppm2jpeg convertit une image au format ppm ou pgm en une image jpeg.
Il prend comme seul parametre obligatoire le nom de l'image PPM a convertir, d'extension .ppm ou .pgm. Seules les images au format PPM P6 ou PGM P5 sont acceptees.

En sortie une image au format JPEG sera generee, encodee en mode JFIF, baseline sequentiel, DCT, Huffman, de meme nom que l'image d'entree et d'extension .jpg.

Pour générer l'executable :

```sh
$ make
```

### Options

| Plugin | README |
| ------ | ------ |
| - - help | Afficher la liste des options acceptees |
| - - outfile=sortie.jpg | Redefinir le nom du fichier de sortie |
| - - sample=h1xv1,h2xv2,h3xv3 | Definir les facteurs d'echantillonnage hxv des trois composantes de couleur |
| - - verbose | Pour avoir un retour sur la generation du fichier |

