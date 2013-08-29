library("XML")

year_2007="/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/2007/graphml_lf_network_2007.graphml"

xml_2007=xmlInternalTreeParse(year_2007)

#first get all nodes and their associated ids
nodes = getNodeSet(xml_2007,"/graphml//graph")




