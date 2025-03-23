# Systemy operacyjne 2 - projekt 1

## Opis 

Tematem projektu była implementacja rozwiązania problemu ucztujących filozofów.

Dla każdego filozofa tworzony jest wątek, a filozof przyjmuje jeden z trzech stanów. W programie sekcjami krytycznymi jest moment w, którym wątek filozofa próbuje działać na widelca, podnieść je lub odłożyć. W celu uniknięcia wyścigu zastosowano mutex, który pozwala na to że w jednej chwili tylko jeden filozof znajduje się w sekcji krytycznej, kiedy filozof kończy jeść, wybudza wątki filozofów z jego lewej i prawej strony, które oczekują na możliwość jedzenia.

## Kompilacja i uruchamianie

Program został skompilowany w systemie macos, i powinien działać na systemach unix-like, kompilację uruchamiamy komendą make.

Program uruchamiamy z argumentem, który określa ilość filozofów.

```bash
$ ./out/main <liczba_filozofów>
```

Przykładowo dla 7 filozofów:

```bash
$ ./out/main 7
```
