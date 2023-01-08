## Wstęp
Sync - narzędzie do synchronizacji plików pomiędzy maszynami

Autor: Bartłomiej Moroz

## Krótka instrukcja obsługi

### Konfiguracje
Tworzenie konfiguracji: skrót `Ctrl+N` lub menu `File->New Configuration`.
Należy podać nazwę (Name), katalog lokalny (Root A), katalog zdalny (Root B), adres maszyny zdalnej (Address/Hostname) i użytkownik maszyny zdalnej (Username).

Wybieranie konfiguracji: skrót `Ctrl+O` lub przycisk `Change Configuration` lub menu `File->Change Configuration`.

### Wykrywanie zmian / Skanowanie
Uruchamianie skanowania: skrót `Ctrl+R` lub przycisk `Scan directories` lub menu `Edit->Scan directories`.

Uwierzytelnianie użytkownika następuje automatycznie. Jeżeli wymagane jest hasło, wyzwanie, lub klucz jest chroniony hasłem, użytkownik zostanie poproszony o wpisanie.

Pliki mogą znajdować się w następujących stanach:
* New - istnieje plik, którego nie było podczas ostatniej synchronizacji
* Absent - brak pliku
* Clean - brak zmian zawartości od ostatniej synchronizacji
* Dirty - zmiany zawartości od ostatniej synchronizacji
* Changed - stan pliku zmienił się pomiędzy skanowaniem a synchronizacją

Program automatycznie sugeruje domyślne akcje:
* Left to Right (oznaczone ==>>) - propaguj zmiany z maszyny lokalnej na zdalną
* Right to Left (oznaczone <<==) - propaguj zmiany z maszyny zdalnej na lokalną
* Ignore - nie rób nic
* Fast Forward - zaktualizuj sam wpis w historii pliku
* Conflict - wystąpił konflikt, żadna akcja nie może zostać podjęta (oprócz rozwiązania konfliktu)
* Resolve - propaguj zmiany po rozwiązanym konflikcie do obu maszyn

Użytkownik może ręcznie wybrać akcję, zaznaczając wiersze listy i wybierając akcję z menu `Actions` lub korzystając z przycisków `Left to Right`, `Ignore`, `Right to Left`, `Resolve Conflict` lub korzystając ze skrótów klawiszowych (w szczególności klawiszy strzałek).

### Zasady ignorowania
Użytkownik może zdefiniować zasady ignorowania ścieżek w plikach `.SyncBlackList` i `.SyncWhiteList` znajdujących się w korzeniu katalogu. Plik `.SyncBlackList` definiuje ścieżki, które będą zawsze pomijane podczas skanowania (w konsekwencji, program nie będzie świadom o ich istnieniu). Plik `.SyncWhiteList` definiuje wyjątki od poprzednich reguł.

Znak `*` oznacza "ciąg znaków dowolnej długości".

Przykładowe zasady:
```
node_modules/*
*.tmp
file.ignore.me
```

### Synchronizacja
Rozpoczynanie synchronizacji: skrót `Ctrl+G` lub `Enter` lub przycisk `Go Synchronize!` lub menu `Edit->Go Synchronize!`. Jeżeli użytkownik nie zaznaczył żadnych plików, synchronizowane są wszystkie, w przeciwnym wypadku tylko te zaznaczone. Pliki o dużym rozmiarze (>50MB) są kompresowane przed wysłaniem.

Program tuż przed propagacją zmian sprawdza szybko stan pliku (porównuje tylko rozmiar i czas modyfikacji). Jeżeli wykryto zmianę, synchronizacja tego pliku jest anulowana (kolumna Progress jest ustawiana na `Canceled`) i musi zostać przeskanowany ponownie.

Jeżeli z jakiegokolwiek powodu synchronizacja pliku nie zakończyła się sukcesem, kolumna Progress jest ustawiana na `Failed`. Bezpieczne jest ponawianie synchronizacji od razu, tzn. nie następuje utrata danych. Dla plików, których synchronizacja jest udana, kolumna Progress jest ustawiana na `Success` i plik na obu maszynach ma status `Clean`.

### Konflikty
Za konflikt uważana jest sytuacja, gdy zawartość pliku jest różna na obu maszynach i w historii synchronizacji. Program domyślnie nie będzie synchronizował takich plików, chyba, że użytkownik ręcznie wybierze akcje lub zdecyduje się rozwiązać konflikt za pomocą przycisku `Resolve Conflict` (lub skrót `Shift+C`, lub menu `Actions->Resolve Conflict with a tool`). Użytkownik może wtedy wybrać jedną (lub stworzyć nową) ze zdefiniowanych zasad lub tryb Auto, który automatycznie dostosuje zasadę do pliku na podstawie nazwy.

Zasady składają się z nadanej przez użytkownika nazwy, reguły dopasowania ścieżki i polecenia do wykonania. Reguły dopasowania ścieżki są analogiczne do tych opisanych w sekcji Zasady ignorowania. Przykład: `*.txt` dopasuje się do każdego pliku z rozszerzeniem `.txt`. Polecenie może być dowolnym poleceniem powłoki, gdzie odwołujemy się do konfliktujących plików za pomocą symboli `$LOCAL` i `$REMOTE`. Przykład: `meld $LOCAL $REMOTE`. 

Po wybraniu zasady, program pobierze plik z maszyny zdalnej, utworzy tymczasowe pliki w katalogu `~/.sync/tmp` (odpowiednio nazwane `xxx.SyncLOCAL` i `xxx.SyncREMOTE`, gdzie `xxx` to hash ścieżki) i wykona polecenie skojarzone z zasadą. Program będzie oczekiwał na wykonanie się polecenia (np. zamknięcie zewnętrznego narzędzia). Po rozwiązaniu konfliktu przez użytkownika należy dokonać synchronizacji zmian (patrz sekcja Synchronizacja). Program pozwoli zsynchronizować zmiany tylko, gdy powstałe tymczasowe pliki mają identyczną zawartość.

## Zależności:

* easyloggingcpp: https://github.com/amrayn/easyloggingpp (MIT)
* fmtlib: https://github.com/fmtlib/fmt (MIT)
* googletest: https://github.com/google/googletest (BSD-3-Clause)
* libssh: https://www.libssh.org (LGPL)
* SQLiteCpp: https://github.com/SRombauts/SQLiteCpp (MIT)
* uuid: https://linux.die.net/man/3/uuid (Linux lib)
* wxWidgets: https://www.wxwidgets.org (wxWindows Library Licence)
* xxHash: https://cyan4973.github.io/xxHash/ (BSD 2-Clause)
* Zstandard: https://github.com/facebook/zstd (BSD/GPLv2)
