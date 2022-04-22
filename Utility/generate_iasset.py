from distutils.log import error
import uuid
import sys

if len(sys.argv) < 2:
	error("Wrong usage!\nSpecify a file to generate an .iasset file for.\n\ngenerate_iasset.py <filename>\n")
	exit()

path = sys.argv[1]

file = path.split("\\")[-1]
name, extension = tuple(file.split(".", 1))

try:
	type = {
		'png': 'Ion.Image',
		'dae': 'Ion.Mesh'
	}[extension]
except:
	type = input("File extension unknown, specify a type: ")

guid = uuid.uuid4()

asset = f"""<IonAsset>
	<Info type="{type}" guid="{guid}" />
	<ImportExternal path="{file}" />
</IonAsset>
"""

assetDir = "\\".join([d for d in path.split('\\') if d != file])
assetPath = f"{assetDir}\\{file.split('.')[0]}.iasset"

with open(assetPath, 'w') as file:
	file.write(asset)
