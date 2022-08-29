/*
 * main.c
 *
 *  Created on: 21 июн. 2022 г.
 *      Author: Ruslan
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define TABLE_START_POINT 1

#define STRING_MAX_SIZE 255

#define PIN_TABLE_FIELD_MAX_SIZE 255
#define PIN_TABLE_COLUMN_MAX_SIZE 255
#define PIN_TABLE_LINE_MAX_SIZE 255

#define GPIO_PORT_COLUMN 0
#define GPIO_PIN_COLUMN 1
#define GPIO_MODE_COLUMN 2
#define GPIO_OTYPE_COLUMN 3
#define GPIO_OSPEED_COLUMN 4
#define GPIO_PUPD_COLUMN 5
#define GPIO_AF_COLUMN 6
#define PIN_TABLE_RIGHT_SHIFT 7

#define GPIO_NAME_COLUMN_PORT_SIZE 2

#define GPIO_PORT_COLUMN_PREFIX "GPIO"

#define GPIO_PIN_COLUMN_PREFIX "GPIO_PIN_"
#define GPIO_PIN_COLUMN_PREFIX_SIZE 9

#define GPIO_MODE_IN "GPIO_MODE_IN"
#define GPIO_MODE_OUT "GPIO_MODE_OUT"
#define GPIO_MODE_AF "GPIO_MODE_AF"
#define GPIO_MODE_AN "GPIO_MODE_AN"

#define GPIO_OTYPE_DEFAULT "GPIO_OTYPE_PP"

#define GPIO_OSPEED_DEFAULT "GPIO_OSPEED_VERY_HIGH"

#define GPIO_PUPD_DEFAULT "GPIO_PUPD_NONE"

#define GPIO_AF_COLUMN_PREFIX "GPIO_AF_"

#define GPIO_AF_DEFAULT "0"

//структура позиции колонок заголовка
typedef struct {
	size_t position;
	size_t name;
	size_t type;
	size_t signal;
	size_t label;
	size_t af0;
} pin_header_t;

//поле
typedef struct {
	char field[PIN_TABLE_FIELD_MAX_SIZE];
} pin_column_t;

//строка
typedef struct {
	pin_column_t column[PIN_TABLE_COLUMN_MAX_SIZE];
} pin_line_t;

//таблица
typedef struct {
	pin_header_t header;
	pin_line_t line[PIN_TABLE_LINE_MAX_SIZE];
	size_t gpio_counter;
	size_t gpio_count;
	size_t lines_counter;
	size_t lines_count;
	size_t columns_counter;
	size_t columns_count;
} pin_table_t;

//структура координат искомого блока
typedef struct {
	size_t shift;
	size_t size;
} strcspn_strspn_t;

static const char gpio_letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char gpio_numeral[] = "1234567890";

pin_table_t pin_table;

void string_handler(pin_table_t *table, char *string) {
	//разделение строки на отдельные строки-столбцы
	table->columns_counter = PIN_TABLE_RIGHT_SHIFT;
	//разделитель
	static const char comp[] = { ',' };
	char *field;
	field = strtok(string, comp);
	while (field != NULL) {
		//копирование строк-столбцов в таблицу
		strcpy(pin_table.line[pin_table.lines_count].column[table->columns_counter].field, field);
		table->columns_counter++;
		field = strtok(NULL, comp);
	}
	table->columns_count = table->columns_counter - PIN_TABLE_RIGHT_SHIFT;
}

void table_gpio_quote_remove(pin_table_t *table, int field_point, int pin_table_line) {
	//удаление пары кавычек из поля
	char *quote_point;
	quote_point = NULL;
	quote_point = strchr(table->line[pin_table_line].column[field_point].field, '"');
	if (quote_point != NULL) {
		memmove(&quote_point[0], &quote_point[1], strlen(&quote_point[1]) + 1);
	}
	quote_point = NULL;
	quote_point = strchr(table->line[pin_table_line].column[field_point].field, '"');
	if (quote_point != NULL) {
	    memmove(&quote_point[0], &quote_point[1], strlen(&quote_point[1]) + 1);
	}
}

void table_gpio_slash_to_space(pin_table_t *table, int field_point, int pin_table_line) {
	//замена слэша пробелом в поле
	char *slash_point = table->line[pin_table_line].column[field_point].field;
	while ((slash_point = strchr(slash_point, '/')) != NULL) {
		*slash_point = ' ';
	}
}

/*
void table_gpio_field_erase(pin_table_t *table, int field_point, int pin_table_line) {
	size_t position_size = strlen(table->line[pin_table_line].column[field_point].field);
		memset(table->line[pin_table_line].column[field_point].field, 0, position_size);
}
*/

strcspn_strspn_t strcspn_strspn(const char * string1, const char * string2) {
	size_t shift = strcspn(string1, string2);
	size_t size = strspn(&string1[shift], string2);
	return (strcspn_strspn_t){shift,size};
}

void table_gpio_name_handler(pin_table_t *table) {
	strcspn_strspn_t name_letters = strcspn_strspn(table->line[table->lines_counter].column[table->header.name].field, gpio_letters);
	strcspn_strspn_t name_numeral = strcspn_strspn(table->line[table->lines_counter].column[table->header.name].field, gpio_numeral);

	//проверка длины имени порта
	if(name_letters.size == GPIO_NAME_COLUMN_PORT_SIZE) {
		sprintf(table->line[table->lines_counter].column[GPIO_PORT_COLUMN].field, "%s%c", GPIO_PORT_COLUMN_PREFIX,
				table->line[table->lines_counter].column[table->header.name].field[name_letters.shift + 1]);
	} else {
		sprintf(table->line[table->lines_counter].column[GPIO_PORT_COLUMN].field, "%s%s", GPIO_PORT_COLUMN_PREFIX,"_ERR");
	}
	if(name_numeral.size) {
		//заполнение префикса пина
		//GPIO_PIN_COLUMN_PREFIX
		strcat(table->line[table->lines_counter].column[GPIO_PIN_COLUMN].field, GPIO_PIN_COLUMN_PREFIX);
		//копирование номера пина
		strncat(table->line[table->lines_counter].column[GPIO_PIN_COLUMN].field,
				&table->line[table->lines_counter].column[table->header.name].field[name_numeral.shift], name_numeral.size);
	}

	table->gpio_count++;
}

void table_gpio_input_handler(pin_table_t *table) {
	//заполнение полей параметров
	sprintf(table->line[table->lines_counter].column[GPIO_MODE_COLUMN].field, "%s", GPIO_MODE_IN);
	sprintf(table->line[table->lines_counter].column[GPIO_OTYPE_COLUMN].field, "%s", GPIO_OTYPE_DEFAULT);
	sprintf(table->line[table->lines_counter].column[GPIO_OSPEED_COLUMN].field, "%s", GPIO_OSPEED_DEFAULT);
	sprintf(table->line[table->lines_counter].column[GPIO_PUPD_COLUMN].field, "%s", GPIO_PUPD_DEFAULT);
	sprintf(table->line[table->lines_counter].column[GPIO_AF_COLUMN].field, "%s%s", GPIO_AF_COLUMN_PREFIX,
			GPIO_AF_DEFAULT);
	//подстановка порта и номера пина в поля имени и позиции
	table_gpio_name_handler(table);
}

void table_gpio_output_handler(pin_table_t *table) {
	//заполнение полей параметров
	sprintf(table->line[table->lines_counter].column[GPIO_MODE_COLUMN].field, "%s", GPIO_MODE_OUT);
	sprintf(table->line[table->lines_counter].column[GPIO_OTYPE_COLUMN].field, "%s", GPIO_OTYPE_DEFAULT);
	sprintf(table->line[table->lines_counter].column[GPIO_OSPEED_COLUMN].field, "%s", GPIO_OSPEED_DEFAULT);
	sprintf(table->line[table->lines_counter].column[GPIO_PUPD_COLUMN].field, "%s", GPIO_PUPD_DEFAULT);
	sprintf(table->line[table->lines_counter].column[GPIO_AF_COLUMN].field, "%s%s", GPIO_AF_COLUMN_PREFIX,
			GPIO_AF_DEFAULT);
	//подстановка порта и номера пина в поля имени и позиции
	table_gpio_name_handler(table);
}

void table_gpio_af_handler(pin_table_t *table) {
	//перебор столбцов альтернативных функций
	int pin_alt_func;
	for (pin_alt_func = table->header.af0; pin_alt_func < PIN_TABLE_COLUMN_MAX_SIZE; pin_alt_func++) {
		//замена слэша пробелом в полях альтернативной функции пина
		table_gpio_slash_to_space(table, pin_alt_func, table->lines_counter);
		//поиск совпадения сигнала с альтернативной функцией
		if (strstr(table->line[table->lines_counter].column[pin_alt_func].field,
				table->line[table->lines_counter].column[table->header.signal].field) != NULL)
			break;
	}

	if (pin_alt_func == PIN_TABLE_COLUMN_MAX_SIZE) {
		//альтернативная функция пина не определена
		sprintf(table->line[table->lines_counter].column[GPIO_AF_COLUMN].field, "%s%s", GPIO_AF_COLUMN_PREFIX,"ERR");
	} else {
		//альтернативная функция пина определена
		//заполнение полей параметров
		sprintf(table->line[table->lines_counter].column[GPIO_MODE_COLUMN].field, "%s", GPIO_MODE_AF);
		sprintf(table->line[table->lines_counter].column[GPIO_OTYPE_COLUMN].field, "%s", GPIO_OTYPE_DEFAULT);
		sprintf(table->line[table->lines_counter].column[GPIO_OSPEED_COLUMN].field, "%s", GPIO_OSPEED_DEFAULT);
		sprintf(table->line[table->lines_counter].column[GPIO_PUPD_COLUMN].field, "%s", GPIO_PUPD_DEFAULT);
		//заполнение поля альтернативной функции
		sprintf(table->line[table->lines_counter].column[GPIO_AF_COLUMN].field, "%s%lu", GPIO_AF_COLUMN_PREFIX,
				pin_alt_func - table->header.af0);
	}

	//подстановка порта и номера пина в поля имени и позиции
	table_gpio_name_handler(table);
}

void table_gpio_type_handler(pin_table_t *table) {
	//удаление кавычек из поля номера пина
	table_gpio_quote_remove(table, table->header.position, table->lines_counter);

	//удаление кавычек из поля имя
	table_gpio_quote_remove(table, table->header.name, table->lines_counter);

	//удаление кавычек из поля сигнал
	table_gpio_quote_remove(table, table->header.signal, table->lines_counter);

	//удаление кавычек из поля пометки
	table_gpio_quote_remove(table, table->header.label, table->lines_counter);

	//тип пина выход
	if (strcmp("\"Input\"", table->line[table->lines_counter].column[table->header.type].field) == 0) {
		table_gpio_input_handler(table);
	} else {
		//тип пина вход
		if (strcmp("\"Output\"", table->line[table->lines_counter].column[table->header.type].field) == 0) {
			table_gpio_output_handler(table);
		} else {
			//остальные пины ввода/вывода
			if (strcmp("\"I/O\"", table->line[table->lines_counter].column[table->header.type].field) == 0) {
				table_gpio_af_handler(table);
			}
		}
	}
}

void table_gpio_printer(pin_table_t *table) {
	if(strstr(table->line[table->lines_counter].column[GPIO_PORT_COLUMN].field, GPIO_PORT_COLUMN_PREFIX) != NULL) {
		printf("GPIO_PIN_CFG(%s, %s, %s, %s, %s, %s, %s), /*%s, %s, %s, %s*/\n",
				table->line[table->lines_counter].column[GPIO_PORT_COLUMN].field,
				table->line[table->lines_counter].column[GPIO_PIN_COLUMN].field,
				table->line[table->lines_counter].column[GPIO_MODE_COLUMN].field,
				table->line[table->lines_counter].column[GPIO_OTYPE_COLUMN].field,
				table->line[table->lines_counter].column[GPIO_OSPEED_COLUMN].field,
				table->line[table->lines_counter].column[GPIO_PUPD_COLUMN].field,
				table->line[table->lines_counter].column[GPIO_AF_COLUMN].field,
				table->line[table->lines_counter].column[table->header.position].field,
				table->line[table->lines_counter].column[table->header.name].field,
				table->line[table->lines_counter].column[table->header.signal].field,
				table->line[table->lines_counter].column[table->header.label].field);
		table->gpio_counter++;
	}
}

void table_handler(pin_table_t *table, char *conf_name) {
	//поиск колонки с указанием имени пина
	for (table->header.position = 0; table->header.position < PIN_TABLE_COLUMN_MAX_SIZE; table->header.position++) {
		if (strcmp(table->line[0].column[table->header.position].field, "\"Position\"") == 0)
			break;
	}

	//поиск колонки с указанием имени пина
	for (table->header.name = 0; table->header.name < PIN_TABLE_COLUMN_MAX_SIZE; table->header.name++) {
		if (strcmp(table->line[0].column[table->header.name].field, "\"Name\"") == 0)
			break;
	}

	//поиск колонки с указанием типа пина
	for (table->header.type = 0; table->header.type < PIN_TABLE_COLUMN_MAX_SIZE; table->header.type++) {
		if (strcmp(table->line[0].column[table->header.type].field, "\"Type\"") == 0)
			break;
	}

	//поиск колонки с указанием сигнала пина
	for (table->header.signal = 0; table->header.signal < PIN_TABLE_COLUMN_MAX_SIZE; table->header.signal++) {
		if (strcmp(table->line[0].column[table->header.signal].field, "\"Signal\"") == 0)
			break;
	}

	//поиск колонки с указанием пометка пина
	for (table->header.label = 0; table->header.label < PIN_TABLE_COLUMN_MAX_SIZE; table->header.label++) {
		if (strcmp(table->line[0].column[table->header.label].field, "\"Label\"") == 0)
			break;
	}

	//поиск первой колонки альтернативных функций пина
	for (table->header.af0 = 0; table->header.af0 < PIN_TABLE_COLUMN_MAX_SIZE; table->header.af0++) {
		if (strcmp(table->line[0].column[table->header.af0].field, "\"AF0\"") == 0)
			break;
	}

	//заголовок таблицы не обработан
	if ((table->header.position == PIN_TABLE_COLUMN_MAX_SIZE) || (table->header.name == PIN_TABLE_COLUMN_MAX_SIZE)
			|| (table->header.type == PIN_TABLE_COLUMN_MAX_SIZE) || (table->header.signal == PIN_TABLE_COLUMN_MAX_SIZE)
			|| (table->header.af0 == PIN_TABLE_COLUMN_MAX_SIZE) || (table->header.label == PIN_TABLE_COLUMN_MAX_SIZE)) {
		printf("Table header process error!\n");
		if (table->header.position == PIN_TABLE_COLUMN_MAX_SIZE)
			printf("Position not found!\n");
		if (table->header.name == PIN_TABLE_COLUMN_MAX_SIZE)
			printf("Name not found!\n");
		if (table->header.type == PIN_TABLE_COLUMN_MAX_SIZE)
			printf("Type not found!\n");
		if (table->header.signal == PIN_TABLE_COLUMN_MAX_SIZE)
			printf("Signal not found!\n");
		if (table->header.label == PIN_TABLE_COLUMN_MAX_SIZE)
			printf("Label bot found!\n");
		if (table->header.af0 == PIN_TABLE_COLUMN_MAX_SIZE)
			printf("AF0 not found!\n");
		printf("Columns: %lu\n", pin_table.columns_count);
		printf("Lines: %lu\n", pin_table.lines_count);
		return;
	}

	//обход таблицы пинов
	for (table->lines_counter = TABLE_START_POINT; table->lines_counter < table->lines_count; table->lines_counter++) {
		table_gpio_type_handler(table);
	}

	//начало массива
	if(conf_name == NULL) {
		printf("static const gpio_pin_cfg_t gpio_init[%lu] = {\n", table->gpio_count);
	} else {
		printf("static const gpio_pin_cfg_t %s[%lu] = {\n", conf_name ,table->gpio_count);
	}
	//обход массива пинов
	for (table->lines_counter = TABLE_START_POINT; table->lines_counter < table->lines_count; table->lines_counter++) {
		table_gpio_printer(table);
	}
	//конец массива
	printf("};\n");

	//проверка количества элементов
	if(table->gpio_counter == table->gpio_count) {
		printf("/*Elements count OK*/");
	} else {
		printf("/*Elements count ERR*/");
	}
}

int main(int argc, char **argv) {
	//проверка количества аргументов
	if (argc < 2) {
		printf("too few arguments\n");
		return 0;
	}
	if (argc > 3) {
		printf("too many arguments\n");
		return 0;
	}

	char *file_name = argv[1];

	char string[STRING_MAX_SIZE];

	memset(string, 0, STRING_MAX_SIZE);
	memset(&pin_table, 0, sizeof(pin_table_t));

	//осторожно, двери открываются
	FILE *file = fopen(file_name, "r");

	if (file == NULL) {
		printf("Error open file");
		return -1;
	}

	while (fgets(string, STRING_MAX_SIZE, file)) {
		string_handler(&pin_table, string);
		pin_table.lines_count++;
	}
	table_handler(&pin_table, argv[2]);

	//осторожно, двери закрываются
	fclose(file);

	return 0;
}

