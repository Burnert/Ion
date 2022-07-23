import re

full_enum = ""

with open("./input_enum.txt") as file:
	full_enum = "".join(file.readlines())

type_name = re.search("(?<=enum class )\w[\w\d]+", full_enum)
if not type_name:
	pass

type_name = type_name.group(0)

enum_body = full_enum[full_enum.find("{"):]

values = re.findall("\w[\w\d]+(?=[,=\n]| =)", enum_body)

with open("./output_enum_parser.txt", "w") as file:
	file.write(f"template<>\nstruct TEnumParser<{type_name}>\n{{\n")
	file.write(f"\tENUM_PARSER_TO_STRING_BEGIN({type_name})\n")
	for val in values:
		file.write(f"\tENUM_PARSER_TO_STRING_HELPER({val})\n")
	file.write("\tENUM_PARSER_TO_STRING_END()\n\n")
	file.write(f"\tENUM_PARSER_FROM_STRING_BEGIN({type_name})\n")
	for val in values:
		file.write(f"\tENUM_PARSER_FROM_STRING_HELPER({val})\n")
	file.write("\tENUM_PARSER_FROM_STRING_END()\n};\n")
