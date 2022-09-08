from enum import Enum

class EUsage(Enum):
	INTERFACE_DECLARATION = 0
	OVERRIDE_DECLARATION = 1
	IMPLEMENTATION = 2

fundamental_types = [
	"bool",
	"int8",
	"int16",
	"int32",
	"int64",
	"uint8",
	"uint16",
	"uint32",
	"uint64",
	"float",
	"double",
]

def generate_archive_serialize_functions(subclass_name, usage=EUsage.INTERFACE_DECLARATION):
	final_output = ""

	if usage == EUsage.INTERFACE_DECLARATION:
		for type in fundamental_types:
			final_output += f"virtual void Serialize({type}& value) = 0;\n"
		final_output += "\n"
		for type in fundamental_types:
			final_output += f"""FORCEINLINE Archive& operator<<({type}& value)
{{
	Serialize(value);
	return *this;
}}

"""

	if usage == EUsage.OVERRIDE_DECLARATION:
		for type in fundamental_types:
			final_output += f"virtual void Serialize({type}& value) override;\n"
		final_output += "\n"

	elif usage == EUsage.IMPLEMENTATION:
		for type in fundamental_types:
			final_output += f"""void {subclass_name}::Serialize({type}& value)
{{
	Serialize(&value, sizeof({type}));
}}

"""

	return final_output

if __name__ == "__main__":
	subclass_name = input("Enter subclass name: ")
	interface_declarations = generate_archive_serialize_functions(subclass_name, EUsage.INTERFACE_DECLARATION)
	override_declarations = generate_archive_serialize_functions(subclass_name, EUsage.OVERRIDE_DECLARATION)
	implementations = generate_archive_serialize_functions(subclass_name, EUsage.IMPLEMENTATION)
	
	with open("generated_functions.txt", "w") as file:
		file.write("// INTERFACE DECL:\n\n")
		file.write(interface_declarations)
		file.write("// OVERRIDE DECL:\n\n")
		file.write(override_declarations)
		file.write("// IMPLEMENTATION:\n\n")
		file.write(implementations)
