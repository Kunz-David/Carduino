\page navod_k_pouziti Návod k použití
\tableofcontents

# Ovládání Carduina

[Ukázka ovládání](https://youtu.be/5XPRxj0TPuw)

Carduino má velice jednoduché ovládání. Na ovladači můžeme ovládát rychlost a směr pohybu autíčka. K tomu nám také napomáhají ultrazvukové sensory (sonary), které kotrolují Carduino může ohrozit blížící se překážka.

## Automatická asistence při jízdě

### Automatické zastavení a zpomalení:
Autíčko umí automaticky [zpomalit](\ref slow)(, když je ve vzdálenosti 35-20cm od překážky) a [zastavit](\ref stop)(, když ve vzdálenosti 20-1cm od překážky). Autíčko nezpomalí ani nezastaví pokud couvá.

### Potlačení změny rychlosti:
Automatické zpomalení i zastavení se dá potlačit stiknutím jakéhokoliv tlačítka pohybu (1-6, #, * a šipky). Potlačení trvá dokud autíčko nevyjede z dosahu zpomalení, nebo zastavení, anebo pokud vyjede z dosahu a uběhne 5s od původního potlačení.

### Zvukový signál:
Pokud se před autíčkem vyskytuje překážka, autíčko před ní varuje pípáním.

\image html IR_remote.jpg "Ovladač" width=200px

### "Neslyšitelné" překážky
Autíčko nerozpozná překážku pokud se do senzorů neodrazí zpátky vyslané ultrazvuky.

\image html No_obstacle_found.jpg "neregistrovaná překážka" width=70%

K autíčku by se v tomto případě pravděpodobně odrazil zpět jen levý (zelený) ultrazvuk, který ovšem nahlásí vzdálenou překážku a autíčko nezpomalí ani nezastaví.

## Tabulka tlačítek

<table>
    <tr>
        <th>Tlačítko</th>
        <th>Funkce</th>
    </tr>
    <tr>
        <td>&larr;</td>
        <td>Zatočit vlevo.</td>
    </tr>
    <tr>
        <td>&uarr;</td>
        <td>Jet rovně.</td>
    </tr>
    <tr>
        <td>&rarr;</td>
        <td>Zatočit vpravo.</td>
    </tr>
    <tr>
        <td>&darr;</td>
        <td>Couvat.</td>
    </tr>
    <tr>
        <td>*</td>
        <td>Zpomalit.</td>
    </tr>
    <tr>
        <td>#</td>
        <td>Zrychlit.</td>
    </tr>
    <tr>
        <td>1</td>
        <td>1. rychostní stupeň.</td>
    </tr>
    <tr>
        <td>2</td>
        <td>2. rychostní stupeň.</td>
    </tr>
    <tr>
        <td>3</td>
        <td>3. rychostní stupeň.</td>
    </tr>
    <tr>
        <td>4</td>
        <td>4. rychostní stupeň.</td>
    </tr>
    <tr>
        <td>5</td>
        <td>5. rychostní stupeň.</td>
    </tr>
    <tr>
        <td>6</td>
        <td>6. rychostní stupeň.</td>
    </tr>
    <tr>
        <td>7</td>
        <td>Vypsat určitá interní data na "Serial" monitor.</td>
    </tr>
    <tr>
        <td>8</td>
        <td>Zapnout/vynout zvuk překážky.</td>
    </tr><tr>
        <td>9</td>
        <td>Volné tlačítko pro možné rozšíření.</td>
    </tr>
    <tr>
        <td>0</td>
        <td>Volné tlačítko pro možné rozšíření.</td>
    </tr>
</table>