# SearchServer
Поисковый сервер, созданный в процессе обучения в Яндекс.Пратикуме. 
Позволяет создать объект типа SearchServer, конструктор с поддержкой добавления соп-слов.
Функционал: 
- AddDocument Добавление документа по идентификационному номеру, строке запроса, статус и рейтинг документа.
- FindTopDocuments Поиск документов по строке-запросу и критерию и вывод 5 найденных докментов с макмимальной релевантностью или рейтингом в порядке убывания.
- MatchDocument Поиск документов по строке-запросу и идентификационному номеру и вывод совпадающих документов.
- RemoveDocument Удаление догумента по идентификационному ноиеру.

Поддержка:
- Парсинг слов входной очереди, с учетом минус-слов и некорректных символов.
- Вывод информации по страницам.

Используются асинхронные вычисления и параллельные алгоритмы.

MS Visual Studio 2019, C++20.

Исправить: тип string_view в добавлении документа.
