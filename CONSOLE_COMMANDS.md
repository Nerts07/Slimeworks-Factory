# Как добавить новую команду в developer console

Консоль использует декларативный список команд через X-macro. Новая команда добавляется в один список, а таблица, autocomplete, `help` и проверка количества параметров подключаются автоматически.

## 1. Добавь описание команды

Открой файл:

```text
src/console/console_commands.def
```

Добавь строку по формату:

```c
CONSOLE_COMMAND(
    heal,
    "Restore player health",
    "heal <amount>",
    "amount: positive number",
    1,
    1,
    CommandHeal
)
```

Параметры строки:

| Поле | Назначение |
| --- | --- |
| `heal` | Имя команды, которое вводит игрок. |
| `"Restore player health"` | Описание в списке подсказок и `help`. |
| `"heal <amount>"` | Полный пример синтаксиса. |
| `"amount: positive number"` | Подсказка по параметрам. |
| `1` | Минимальное количество аргументов после имени команды. |
| `1` | Максимальное количество аргументов. |
| `CommandHeal` | Имя C-функции обработчика. |

Для команды без параметров используй `0, 0`:

```c
CONSOLE_COMMAND(day, "Set daytime", "day", "", 0, 0, CommandDay)
```

Для необязательного параметра используй диапазон, например `0, 1`.

## 2. Объяви обработчик

Открой:

```text
src/console/console_commands.c
```

В верхней части файла добавь объявление:

```c
static bool CommandHeal(
    ConsoleExecutionContext *context,
    int argc,
    const char *argv[],
    char *message,
    int messageSize
);
```

Затем добавь реализацию:

```c
static bool CommandHeal(ConsoleExecutionContext *context,
                        int argc,
                        const char *argv[],
                        char *message,
                        int messageSize)
{
    (void)argc;

    char *end = NULL;
    float amount = strtof(argv[0], &end);
    if (!end || *end != '\0' || amount <= 0.0f) {
        snprintf(message, (size_t)messageSize, "Usage: heal <positive amount>");
        return false;
    }

    /* Здесь находится действие команды. */
    (void)context;
    (void)amount;
    snprintf(message, (size_t)messageSize, "Healed by %.1f", amount);
    return true;
}
```

## 3. Что доступно через `context`

`ConsoleExecutionContext` передаёт основные игровые системы:

```c
context->state          // GameState: ресурсы, здания, контракты
context->level          // Level: мир и landmark-позиции
context->playerSystem   // PlayerSystem: игрок, камера и ввод
context->gameplay       // GameplaySystem: состояние режима строительства
context->console        // сама консоль: лог и очистка
```

Пример выдачи ресурса:

```c
EconomyAdd(&context->state->economy, RESOURCE_SLIME, amount);
```

Пример вывода дополнительной строки:

```c
DeveloperConsoleWrite(context->console, "Current slime: %.1f",
                      context->state->economy.amount[RESOURCE_SLIME]);
```

Обработчик должен вернуть:

- `true`, если команда выполнена;
- `false`, если произошла ошибка или параметры неверные.

Текст из `message` автоматически появится в консоли после выполнения команды.

## 4. Аргументы команды

Аргументы приходят в `argv` без имени команды.

Для команды:

```text
give slime 100
```

будет:

```c
argc == 2
argv[0] == "slime"
argv[1] == "100"
```

Количество аргументов проверяется автоматически по `minArgs` и `maxArgs` в `.def`. Если количество неверное, handler даже не будет вызван — пользователь увидит строку `Usage`.

Для чисел используй `strtof` или `strtol` и обязательно проверяй `end`:

```c
char *end = NULL;
long value = strtol(argv[0], &end, 10);
if (!end || *end != '\0') {
    snprintf(message, (size_t)messageSize, "Value must be an integer");
    return false;
}
```

## 5. Как проверить новую команду

Собери игру:

```sh
cd /Users/isa/Documents/C-Projects/RaylibGame
make clean && make
```

Запусти игру через VS Code, нажми `F4` и проверь:

```text
help
help heal
heal 25
```

Проверь также:

- команда появляется в autocomplete после ввода начала имени;
- `Tab` завершает имя;
- неправильное число аргументов показывает `Usage`;
- ошибка handler возвращает понятное сообщение;
- `Up` и `Down` сохраняют работу истории команд.

## Важное правило архитектуры

Не добавляй новую команду в `game.c`. Не добавляй отдельный `if (strcmp(command, ...))` в консоль. Достаточно:

1. одной строки в `src/console/console_commands.def`;
2. объявления handler;
3. реализации handler в `src/console/console_commands.c`.

Так autocomplete, `help`, синтаксис и диспетчеризация остаются общими для всех команд.
