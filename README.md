## Compilazione:
```bash
cmake -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

## Esecuzione:
```bash
cd build

./tappa01
./tappa02
./tappa03
./tappa04
./tappa05
./tappa06
./tappa07
./tappa08
./tappa09
./tappa10
```
Le tappe non richiedono argomenti aggiuntivi da riga di comando. 

## Comandi interfaccia utente:

### Mouse

Comandi uguali dalla tappa 2 in poi.

| Comando | Azione | Da |
|---|---|---|
| Trascinamento con tasto sinistro | Ruota la camera | Tappa02 |
| `Ctrl` + mouse su/giù | Avvicina o allontana la camera | Tappa02 |

### Tastiera

I comandi seguenti si possono eseguire dalla tappa 4. 

| Comando | Azione |
|---|---|
| `P` | Pausa / riprendi |
| `↑` / `↓` | Accelera / rallenta il tempo |
| `R` | Velocità del tempo a 1× |
| `O` | Mostra / nascondi le orbite |

Per selezionare un corpo celeste:

| Tappe | Comandi |
|---|---|
| Tappa04 – Tappa06 | `0` Sole, `1` … `8` i pianeti da Mercurio a Nettuno |
| Tappa07 | `0` Sole, `1` … `7` da Mercurio a Urano, `8` la Luna, `9` Nettuno |
| Tappa08 in poi | `0` Sole, `1` … `8` i pianeti da Mercurio a Nettuno; `S` scorre i satelliti del corpo inquadrato e torna al pianeta |

Dalla Tappa08 per selezionare i satelliti: 
si seleziona il pianeta con il numero e poi si preme `S` per passare alle sue lune, una alla volta. Premendo `5` e
poi `S` si scorrono Io, Europa, Ganimede e Callisto, e alla quinta pressione si torna a Giove.

Il corpo inquadrato è indicato nel titolo della finestra, e l'elenco completo dei
comandi viene stampato sul terminale all'avvio.

### note

La Tappa01 non ha comandi interattivi. La Tappa03, la Tappa05 e la Tappa06 non aggiungono comandi. 
La finestra è ridimensionabile in tutte le tappe.
