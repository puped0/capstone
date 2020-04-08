import xml.etree.ElementTree as ET
tree = ET.parse('country_data.xml')
root = tree.getroot()

for n in root.iter('neighbor'):
	print(n.attrib)


