# png-file-with-text
Создание png файла с нарисованным в нём текстовым сообщением с помощью шрифта  *.ttf

## Описание программы
1)	Программа компилируется с помощью gcc и системы makefile в ОС Linux или WSL;
2)	Программа работает в нескольких версиях:
     * **static** - статическая компоновка всех трех библиотек;
     * **dynamic** – динамическая загрузка всех трех библиотек в начале работы с помощью dlload;
     * **blob** – реализация всего функционала в виде блоба и загрузка его с диска с помощью загрузчика elf-loader;
3) Система Makefile поддерживает следующие команды:
     * **make static** – компиляция исходников в версии со статической компоновкой;
     * **make dynamic** – компиляция исходников в версии с динамической компоновкой;
     * **make blob** – компиляция исходников в версии с компоновкой в виде блоба;
     * **make clean** – удаление всех бинарных файлов;
     * **make all** – удаление старых бинарных файлов и компиляция исходников в версиях static, dynamic и blob.
4) Программа в независимости от типа сборки по результатам своей работы генерирует png файл с нарисованным текстовым сообщением в нем;
5) Файл со шрифтом, имя png файла и печатаемый текст поступают в программу из аргументов командной строки.
