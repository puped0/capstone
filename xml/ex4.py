

import xml.etree.ElementTree as ET
tree = ET.parse('country_data.xml')
root = tree.getroot()

print(root.findall('country'))

for c in root.findall('country'):
	rank = c.find('rank').text
	name = c.get('name')
	print(name, rank)

