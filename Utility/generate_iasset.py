from distutils.log import error
import uuid
import sys

if len(sys.argv) < 2:
	error("Wrong usage!\nSpecify a file to generate an .iasset file for.\n\ngenerate_iasset.py <filename>\n")
	exit()

file = sys.argv[1]

path = file.split("\\")[-1]
name = path.split(".")[0]
extension = path.split(".")[-1]

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
	<ImportExternal path="{path}" />
</IonAsset>
"""

assetPath = f"{path.split('.')[0]}.iasset"

with open(assetPath, 'w') as file:
	file.write(asset)
