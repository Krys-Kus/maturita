# Maturita
## Triggerbot
Přehodnocení, nejdřív udělat aimbot, jelikož jsem našel 3D souřadnice a úhel pohledu před Crosshairname, takže podmínku aktivace triggerbota můžu udělat locknutí se na bota.

- Plán mít do dvou týdnů hotovo
- Zjistit jestli se dívám na nepřítele přes CharacterID
- Najít trigger funkci u zbraní, která bude zavolána pokud se jako hráč dívám na nepřítele
  - [x] Adresa CharacterID
    - [x] Rozlišit spoluhráče od nepřítele
  - [ ] Adresa weapon trigger
  - [x] Spojit dohromady a sepsat kód
  - [x] Triggerbot dokončen

## Wallhack
- Chtěl bych wallhack dělat po triggerbotu, ale je možné, že budu nejdříve dělat aimbot
- Vužít adresu CharacterID a rozšířit jí na celé lobby
- Najít adresu pozice nepřítele, okolo které můžu vykreslit obdelník na jejich zvýraznění
    - [x] Rozlišení všech hráčů v lobby
    - [x] Adresa pozice nepřátel
    - [ ] Adresa _Draw funkce na zobrazení 2D tvarů
    - [ ] Spojit dohromady a sepsat kód
    - [ ] Wallhack dokončen

## Aimbot
- Využít adresu, která ukládá pozici nepřítele
- Najít adresu, která ukládá moji vlastní pozici, abych ji mohl srovnt s pozicí nejbližšího nepřítele
- Najít adresu, která ukládá informaci o tom, kam se jako hráč dívám
- Sepsat kód, který bude simulovat pohyb myši směrem k nejbližšímu nepříteli
    - [x] Adresa pozici hráče
    - [x] Adresa pohledu/kamery
    - [x] Počítání a srovnávání outputových souřadnic
    - [x] Spojit dohromady a sepsat kód
    - [x] Aimbot dokončen
