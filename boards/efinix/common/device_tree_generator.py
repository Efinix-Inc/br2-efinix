#!/usr/bin/python3

import argparse
import json
import os

DRIVER_FILE = os.path.join(os.path.relpath(os.path.dirname(__file__)), "drivers.json")

# search keyword in soc.h
INTERRUPT = "IO_INTERRUPT"
PERIPHERAL = "CTRL "
IO_SIZE = "CTRL_SIZE"
FREQUENCY = "CLINT_HZ"
BASE_ADDR_PROP = "SYSTEM_BMB_PERIPHERAL_BMB "
FPU = "FPU"
MMU = "MMU"
ICACHE = "ICACHE"
DCACHE = "DCACHE"
CONTROLLER = ["PLIC", "CLINT", "RAM"]
PLIC = "PLIC"
SUPERVISOR = "SUPERVISOR "

def read_file(filename):
    with open(filename, 'r') as f:
        cfg = f.readlines()

    return cfg


def save_file(filename, data):
    with open(filename, 'w') as f:
        f.write(data)


def load_config_file():
    with open(DRIVER_FILE) as f:
        drivers = json.load(f)

    return drivers


def print_node(node):
    print(json.dumps(node, sort_keys = False, indent = 4))


"""
get_value: get a value for a given property string

@prop (str): property string

return: string of value
"""
def get_value(prop):
    # remove tail character
    prop = prop.rstrip('\n')
    props = prop.split()

    return props[len(props) -1]


"""
get_peripheral_properties: get a list of a peripheral's properties

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: list of peripheral's properties for a given peripheral
"""
def get_peripheral_properties(cfg, peripheral):
    props = []

    for line in cfg:
        if peripheral in line:
            props.append(line)
    return props


"""
get_property_value: get the value of peripheral properties from soc.h

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter
@name (str): properties name such as SIZE, INTERRUPT

return: string of the property value
"""
def get_property_value(cfg, peripheral, name):
    value = ''
    props = get_peripheral_properties(cfg, peripheral)

    for prop in props:
        if name in prop:
            value = get_value(prop)
    return value


"""
get_size: get the peripheral allocated memory size

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: string of size of memory allocated for the peripheral in hex
"""
def get_size(cfg, peripheral):
    size = get_property_value(cfg, peripheral, IO_SIZE)
    if not size:
        keyword_size = 'SYSTEM_{0}_IO_{1}'.format(peripheral, IO_SIZE)
        print("Error: Size for {0} is invalid. Expecting {1}".format(peripheral, keyword_size))

    return size


def get_base_address(cfg):
    props = get_peripheral_properties(cfg, BASE_ADDR_PROP)
    for prop in props:
        addr = get_value(prop)
    return addr

"""
get_address: get the address of the peripheral

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: string of address of the peripheral in hex
"""
def get_address(cfg, peripheral):
    addr = get_property_value(cfg, peripheral, PERIPHERAL)
    if not addr:
        keyword_addr = '{0}_{1}'.format(peripheral, PERIPHERAL)
        print("Error: Address for {0} not found. Expecting {1}".format(peripheral, keyword_addr))
    return addr


"""
get_peripheral_base_address: calculate the base address of peripheral

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: string of peripheral base address in hex
"""
def get_peripheral_base_address(cfg, peripheral):
    addr = get_address(cfg, peripheral)
    base = get_base_address(cfg)

    addr = hex(int(addr, 0) - int(base, 0))

    return addr


"""
get_interrupt_id: get interrupt number of peripheral

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: string of interrupt of the peripheral
"""
def get_interrupt_id(cfg, peripheral):
    irq_name = "{0}_{1}".format(peripheral, INTERRUPT)

    irq = get_property_value(cfg, peripheral, irq_name)
    return irq


"""
get_frequency: get the frequency of soc

@cfg (list): raw data of soc.h

return: string of soc's frequency
"""
def get_frequency(cfg):
    freq = ''
    props = get_peripheral_properties(cfg, FREQUENCY)
    for prop in props:
        freq = get_value(prop)

    return freq


def get_status(okay=False):
    if okay:
        return 'status = "okay";'
    else:
        return 'status = "disabled";'


"""
count_peripheral: count the number for the peripheral in the soc

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: number of peripheral in the soc
"""
def count_peripheral(cfg, peripheral):
    count = 0
    p = []

    props = get_peripheral_properties(cfg, peripheral)
    for prop in props:
        if PERIPHERAL in prop:
            p.append(prop)
            count = count + 1

    return count


def get_cpu_count(cfg):
    count = 0

    props = get_peripheral_properties(cfg, SUPERVISOR)
    count = len(props)

    if count == 0:
        count = 1

    return count


"""
get_cache_way: get number of I/D cache way

@cfg (list): raw data of soc.h
@cache_type (str): ICACHE or DCACHE cache

return: number of cache way for I or D cache
"""
def get_cache_way(cfg, core, cache_type):
    cache_type = "{}_WAY".format(cache_type)
    system_core = "SYSTEM_CORES_{}".format(core)
    way = get_property_value(cfg, system_core, cache_type)

    return way


def get_cache_size(cfg, core, cache_type):
    cache_type = "{}_SIZE".format(cache_type)
    system_core = "SYSTEM_CORES_{}".format(core)
    size = get_property_value(cfg, system_core, cache_type)

    return size


def get_cpu_isa(cfg, core):
    isa = "rv32im"
    system_core = "SYSTEM_CORES_{}".format(core)

    value = get_property_value(cfg, system_core, MMU)
    if value == "1":
        # append 'a' for atomic RISCV instruction extension
        isa = "{}a".format(isa)

    value = get_property_value(cfg, system_core, FPU)
    if value == "1":
        # append 'fd' for floating point & double percision RISCV instruction extension
        isa = "{}fd".format(isa)

    return isa


"""
get_cpu_metadata: get metadata of a cpu

@cfg (list): raw data of soc.h
@idx (int): cpu core number

return: dict of a cpu metadata
"""
def get_cpu_metadata(cfg, idx=0):
    node = {}

    core = "core{}".format(idx)
    isa = get_cpu_isa(cfg, idx)
    icache_way = get_cache_way(cfg, idx, ICACHE)
    icache_size = get_cache_size(cfg, idx, ICACHE)
    dcache_way = get_cache_way(cfg, idx, DCACHE)
    dcache_size = get_cache_size(cfg, idx, DCACHE)

    node = {
        "name": "cpu",
        "addr": idx,
        "reg": "reg = <{}>;".format(idx),
        "device_type": 'device_type = "cpu";',
        "compatible": 'compatible = "riscv";',
        "isa": 'riscv,isa = "{}";'.format(isa),
        "tlb": "tlb-split;",
        "status": get_status(okay=True)
    }

    if icache_way and icache_size and dcache_way and dcache_size:
        node.update({
            "icache_way": "i-cache-sets = <{}>;".format(icache_way),
            "icache_size": "i-cache-size = <{}>;".format(icache_size),
            "icache_block_size": "i-cache-block-size = <32>;",
            "dcache_way": "d-cache-sets = <{}>;".format(dcache_way),
            "dcache_size": "d-cache-size = <{}>;".format(dcache_size),
            "dcache_block_size": "d-cache-block-size = <32>;",
        })

    system_core = "SYSTEM_CORES_{}".format(idx)

    value = get_property_value(cfg, system_core, MMU)
    if value:
        node.update({"mmu_type": 'mmu_type = "riscv,sv32";'})

    return node


def dt_address_cells(num):
    return "#address-cells = <{}>;".format(num)


def dt_size_cells(num):
    return "#size-cells = <{}>;".format(num)


def dt_get_clock_frequency(cfg):
    freq = get_frequency(cfg)
    return "clock-frequency = <{}>;".format(freq)


def dt_get_timebase_frequency(cfg):
    freq = get_frequency(cfg)
    return "timebase-frequency = <{}>;".format(freq)


"""
dt_interrupt: return a string of device tree syntax of interrupt

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: string of device tree interrupt syntax
"""
def dt_interrupt(cfg, peripheral):
    out = ''

    irq = get_interrupt_id(cfg, peripheral)
    if irq:
        out = "interrupts = <{}>;".format(irq)

    return out


"""
dt_reg: string of device tree syntax of reg

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: string of device tree reg syntax
"""
def dt_reg(cfg, peripheral):
    out = ''

    addr = get_peripheral_base_address(cfg, peripheral)
    size = get_size(cfg, peripheral)

    if addr and size:
        out = "reg = <{0} {1}>;".format(addr, size)

    return out


"""
dt_compatible: get compatible driver for the peripheral

@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: string of device tree compatible syntax
"""
def dt_compatible(peripheral, controller=False):
    out = ''
    drv = ''
    peripheral = peripheral.lower()

    drivers = load_config_file()

    if controller:
        drivers = drivers['controller']
    else:
        drivers = drivers['drivers']

    if peripheral in drivers:
        compatible = drivers[peripheral]['compatible']
        drv = ', '.join('"{}"'.format(drv) for drv in compatible)
        out = 'compatible = {};'.format(drv)

    return out


"""
dt_get_phandle: get phandle or reference of device node label

@node (dict): device tree node
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: string of phandle for a given peripheral
"""
def dt_get_phandle(nodes, peripheral):
    phandle = ''

    if isinstance(nodes, dict):
        for key in nodes:
            if peripheral in key:
                if 'label' in nodes[key]:
                    label = nodes[peripheral]['label']
                    phandle = "&{}".format(label)
                    return phandle

            else:
                phandle = dt_get_phandle(nodes[key], peripheral)

    return phandle


"""
dt_insert_child_node: insert child node into parent node as nested dict

@parent (dict): parent dictionary of paripheral
@child (dict): child node

return: parent node with nested child node
"""
def dt_insert_child_node(parent, child):
    for key in parent:
        parent[key].update(child)

    return parent


"""
dt_insert_data: insert device tree data into current device tree node

@node (dict): device tree node of a peripheral
@new_data (dict): new data to be inserted into the node
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: device tree node
"""
def dt_insert_data(node, new_data, peripheral):
    if peripheral in node:
        node[peripheral].update(new_data)

    return node


"""
__dt_create_node_str: convert node information to string

@node (dict): node contain device tree metadata such as label, compatible, reg, interrupt
@parent_node (dict): parent of @node

return: string of device tree node
"""
def __dt_create_node_str(node, parent_node):
    out = ''
    output = ''

    if 'version' in node:
        out += node['version']

    if 'include' in node:
        for i in node['include']:
            out += "/include/ \"{}\"\n".format(i)

    if '#include' in node:
        for i in node['#include']:
            out += "#include <{}>\n".format(i)

    out += '\n'

    if 'label' in node:
        if 'name' and 'addr' in node:
            out += "{0}: {1}@{2} {{\n".format(node['label'], node['name'], node['addr'])

        elif 'name' and not 'addr' in node:
            out += "{0}: {1} {{\n".format(node['label'], node['name'])

        else:
            out += "{0} {{\n".format(node['label'])

    else:
        if 'name' and 'addr' in node:
            out += "{0}@{1} {{\n".format(node['name'], node['addr'])

        elif 'name' and not 'addr' in node:
            out += "{} {{\n".format(node['name'])

        else:
            out += ""

    if 'model' in node:
            out += "\t{}\n".format(node['model'])

    if 'device_type' in node:
            out += "\t{}\n".format(node['device_type'])

    if 'addr_cell' in node:
        #out += "\t{}\n".format(node['addr_cell'])
        addr_cell = dt_address_cells(node['addr_cell'])
        out += "\t{}\n".format(addr_cell)

    if 'size_cell' in node:
        #out += "\t{}\n".format(node['size_cell'])
        size_cell = dt_size_cells(node['size_cell'])
        out += "\t{}\n".format(size_cell)

    if 'reg' in node:
        out += "\t{}\n".format(node['reg'])

    if 'compatible' in node:
        out += "\t{}\n".format(node['compatible'])

    if 'ranges' in node:
        out += "\t{}\n".format(node['ranges'])

    # TODO: SPI does not support interrupt
    if not 'spi' in node['name']:
        if 'interrupt' in node:
            out += "\t{}\n".format(node['interrupt'])

            #phandle of interrupt-parent
            phandle = dt_get_phandle(parent_node, 'PLIC')
            if phandle:
                out += "\tinterrupt-parent = <{}>;\n".format(phandle)

    if 'isa' in node:
        out += "\t{}\n".format(node['isa'])

    if 'mmu_type' in node:
        out += "\t{}\n".format(node['mmu_type'])

    if 'icache_size' in node:
        out += "\t{}\n".format(node['icache_size'])

    if 'icache_way' in node:
        out += "\t{}\n".format(node['icache_way'])

    if 'icache_block_size' in node:
        out += "\t{}\n".format(node['icache_block_size'])

    if 'dcache_size' in node:
        out += "\t{}\n".format(node['dcache_size'])

    if 'dcache_way' in node:
        out += "\t{}\n".format(node['dcache_way'])

    if 'dcache_block_size' in node:
        out += "\t{}\n".format(node['dcache_block_size'])

    if 'tlb' in node:
        out += "\t{}\n".format(node['tlb'])

    if 'private_data' in node:
        # private_data (list)
        for p in node['private_data']:
            out += "\t{}\n".format(p)

    if 'clock_cells' in node:
        out += "\t{}\n".format(node['clock_cells'])

    if 'clock_freq' in node:
        out += "\t{}\n".format(node['clock_freq'])

    if 'timebase_freq' in node:
        out += "\t{}\n".format(node['timebase_freq'])

    if 'status' in node:
        out += "\t{}\n".format(node['status'])

    return out


"""
__dt_create_node: convert node information to string

@nodes (dict): input node contain device tree metadata such as label, compatible, reg, interrupt

return: string of device tree node
"""
def __dt_create_node(nodes):
    out = ''

    parent_node = nodes;

    for node in nodes:
        out = __dt_create_node_str(node, parent_node)

    return out


def get_private_data(peripheral, controller=False):
    priv_data = ''
    peripheral = peripheral.lower()
    drv_data = load_config_file()

    if controller:
        drv = 'controller'
    else:
        drv = 'drivers'

    if peripheral in drv_data[drv]:
        if 'private_data' in drv_data[drv][peripheral]:
            priv_data = drv_data[drv][peripheral]['private_data']

    return priv_data


def dt_get_private_data(peripheral, controller=False):
    priv_data = {}
    priv_data['private_data'] = get_private_data(peripheral, controller)

    return priv_data


"""
dt_create_node: create a device tree node for a peripheral

@cfg (list): raw data of soc.h
@peripheral (str): peripheral name such as SPI, I2C. Must be in capital letter

return: dict of device tree peripheral nodes
"""
def dt_create_node(cfg, peripheral):
    node = {}
    nodes = {}

    count = count_peripheral(cfg, peripheral)
    node = {"count": count}

    for i in range(0, count):
        if peripheral in CONTROLLER:
            node_idx = peripheral
            label = peripheral.lower()
            status = get_status(okay=True)
        else:
            node_idx = "{0}_{1}".format(peripheral, i)
            label = "{0}{1}".format(peripheral.lower(), chr(65+i))
            status = get_status(okay=False)

        reg = dt_reg(cfg, node_idx)
        addr = get_peripheral_base_address(cfg, node_idx)

        node = {
            "name": peripheral.lower(),
            "label": label,
            "reg": reg,
            "addr": addr.lstrip('0x'),
            "status": status
        }

        compatible = dt_compatible(peripheral)
        if compatible:
            node.update({"compatible": compatible})

        irq = dt_interrupt(cfg, node_idx)
        if irq:
            node.update({"interrupt": irq})

        priv_data = dt_get_private_data(peripheral)
        if priv_data:
            node.update(priv_data)

        nodes.update({node_idx: node})

    return nodes


def dt_create_node_str(cfg, peripheral):
    out = ''

    nodes =  dt_create_node(cfg, peripheral)
    out =  __dt_create_node(nodes)

    return out


"""
dt_create_parent_node: create parent node such as clock, apb, axi

@cfg (list): raw data of soc.h
@name (str): name of the parent node
@address_cell (int):
@size_cell (int):

return: parent device tree node

"""
def dt_create_parent_node(cfg, name, address_cell, size_cell):
    node = {
        "name": name,
        "addr_cell": address_cell,
        "size_cell": size_cell
    }

    if not 'cpu' in name:
        compatible = dt_compatible('bus')
        node['compatible'] = compatible

    node = {name: node}

    return node


def dt_create_plic_node(cfg):
    plic_metadata = []
    ext = ''
    node = {}
    priv_data = {}

    drivers = load_config_file()
    cpu_count = get_cpu_count(cfg)
    node = dt_create_node(cfg, PLIC)

    for i in range(0, cpu_count):
        ext += "\n\t\t&L{0} 11 &L{1} 9".format(i, i)

    ext = "interrupts-extended = <{}>;".format(ext)
    priv_data = get_private_data('plic')
    priv_data.append(ext)
    plic_metadata = {"private_data": priv_data}
    node = dt_insert_data(node, plic_metadata, PLIC)

    return node


def dt_create_clock_node(cfg):
    name = "clock"
    label = "apbA_clock"

    node = {
        "label": label,
        "name": name,
        "addr": "1",
        "reg": "reg = <1 0>;",
        "compatible": dt_compatible("clock"),
        "clock_cells": "#clock-cells = <0>;",
        "clock_freq": dt_get_clock_frequency(cfg)
    }

    node = {label: node}

    parent_node = dt_create_parent_node(cfg, name, 1, 0)
    parent_node = dt_insert_child_node(parent_node, node)

    return parent_node


def dt_create_interrupt_controller_node():
    name = "interrupt-controller"
    node = {}

    compatible = dt_compatible('plic', controller=True)
    priv_data = get_private_data('plic', controller=True)
    priv_data.append(compatible)

    node = {
        "name": name,
        "private_data": priv_data
    }

    node = {name: node}

    return node


def dt_create_cpu_node(cfg):
    name = "cpus"
    cpu_nodes = {}
    cpu_count = get_cpu_count(cfg)

    timebase_freq = {"timebase_freq": dt_get_timebase_frequency(cfg)}
    cpu_nodes.update(timebase_freq)

    for cpu in range(0, cpu_count):
        # interrupt controller
        intc = dt_create_interrupt_controller_node()
        intc_label = "L{0}".format(cpu)
        intc['interrupt-controller']['label'] = intc_label

        core = "core{}".format(cpu)
        cpu_node = get_cpu_metadata(cfg, cpu)
        cpu_node = dt_insert_child_node({core: cpu_node}, intc)
        cpu_nodes.update(cpu_node)

    parent = dt_create_parent_node(cfg, name, 1, 0)
    parent = dt_insert_child_node(parent, cpu_nodes)

    return parent


def dt_create_memory_node(cfg):
    name = "memory"
    memory_keyword = "DDR_BMB"
    conf = load_config_file()
    # addr use linux start addr
    addr = conf['memory_mapped']['uImage']

    size = get_property_value(cfg, memory_keyword, 'SIZE ')
    size = hex(int(size,0) - int(addr, 0))
    reg = "reg = <{0} {1}>;".format(addr, size)
    mem_node = {
        "name": name,
        "device_type": 'device_type = "memory";',
        "addr": addr.lstrip('0x'),
        "size": size,
        "reg": reg
    }

    mem_node = {name: mem_node}
    return mem_node


def dt_version():
    conf = load_config_file()
    version  = '{}\n\n'.format(conf['dt_version'])
    return version


def dt_model():
    conf = load_config_file()
    model = 'model = "{}";'.format(conf['model'])
    return model


def dt_create_root_node():
    root_node = {
            "root": {
                "name": "/",
                "version": dt_version(),
                "model": dt_model(),
                "addr_cell": 1,
                "size_cell": 1
                }
            }

    return root_node


"""
dt_get_bus_range: get the bus address range

@cfg (list): raw data of soc.h
@bus_name (str): can be PERIPHERAL_BMB, DDR_BMB, AXI_A_BMB

return: string of device tree ranges property
"""
def dt_get_bus_range(cfg, bus_name):
    addr = get_property_value(cfg, bus_name, bus_name + ' ')
    if not addr:
        print("Error: address for {} is invalid".format(bus_name))

    size = get_property_value(cfg, bus_name, "SIZE ")
    if not size:
        keyword_size = '{}_SIZE'.format(bus_name)
        print("Error: size for {0} is invalid. Expecting {1}".format(bus_name, keyword_size))

    ranges = "ranges = <0x0 {0} {1}>;".format(addr, size)
    return ranges


def dt_create_bus_node(cfg, bus_name, bus_label):
    bus_node = dt_create_parent_node(cfg, bus_label, 1, 1)
    bus_range = dt_get_bus_range(cfg, bus_name)
    addr = get_property_value(cfg, bus_name, bus_name + ' ').lstrip('0x')
    bus_node[bus_label].update({'ranges': bus_range})
    bus_node[bus_label].update({'addr': addr})

    return bus_node


def indent(lines, level=1):
    out = ''
    ch = "\t"
    output = lines.split('\n')

    for line in output:
        out += "{0}{1}\n".format(ch*level, line)

    return out


def __dt_parser_nodes_recursive(nodes, parent_node, count):
    out = ''
    out_temp = ''
    count += 1

    for k in nodes:
        if isinstance(nodes[k], dict):
            out_temp = __dt_create_node_str(nodes[k], parent_node)
            out_temp = indent(out_temp, count)
            out += out_temp
            out += __dt_parser_nodes_recursive(nodes[k], parent_node, count)
            out_temp = "};\n"
            out_temp = indent(out_temp, count)
            out += out_temp

    return out

"""
dt_parser_nodes: parser nodes to device tree format

@nodes (dict): nested dict node of paripheral
@parent_node (dict): parent node of @nodes or same level

return: String of nodes in device tree format
"""
def dt_parser_nodes(nodes, parent_node):
    out = ''
    out_temp = ''
    end_braces = True

    for k in nodes:
        n_node = nodes[k]
        out += __dt_create_node_str(n_node, parent_node)
        out1 = __dt_parser_nodes_recursive(n_node, parent_node, 0)

        if out1:
            out1 += "};\n"
            out += out1

        else:
            out += "};\n"

    return out


"""
create_dts_file: create dts file

@cfg (list): raw data of soc.h
@bus_node: node that contain all peripherals connected to it

return: string of nodes
"""
def create_dts_file(cfg, bus_node):
    dts_root_node = dt_create_root_node()
    node = {}
    nodes = {}

    conf = load_config_file()
    inc_file = {
        "include": conf['dts']['include'],
        "#include": conf['dts']['#include']
    }

    dts_root_node['root'].update(inc_file)
    dts_root_node['root'].update(conf['dts']['root'])

    # memory
    mem_node = dt_create_memory_node(cfg)
    if mem_node:
        dts_root_node = dt_insert_child_node(dts_root_node, mem_node)

    dts = dt_parser_nodes(dts_root_node, dts_root_node)

    for k in bus_node:
        n_node = bus_node[k]
        for periph in n_node:
            if isinstance(n_node[periph], dict):
                if 'status' in n_node[periph]:
                    if 'disabled' in n_node[periph]['status']:
                        node = {
                            "name": dt_get_phandle(n_node, periph),
                            "status": get_status(okay=True)
                        }

                        label = n_node[periph]['label']
                        if label in conf['dts']:
                            node.update(conf['dts'][label])

                        nodes.update({periph: node})

    dts += dt_parser_nodes(nodes, nodes)

    return dts


def main():
    out = ''
    board = ''
    path_dts = 'dts'
    path_dts = os.path.join(os.path.relpath(os.path.dirname(__file__)), path_dts)
    output_filename = 'sapphire.dtsi'
    output_filename = os.path.join(path_dts, output_filename)
    output_json = 'sapphire.json'

    dts_filename = 'linux.dts'
    efinix_dir = os.path.join(os.path.relpath(os.path.dirname(__file__)), '..')

    dt_parse = argparse.ArgumentParser(description='Device Tree Generator')
    dt_parse.add_argument('soc', type=str, help='path to soc.h')
    dt_parse.add_argument('board', type=str, help='development kit name such as t120, ti60')
    dt_parse.add_argument('-o', '--outfile', type=str, help='Override output filename. By default is sapphire.dtsi')
    dt_parse.add_argument('-j', '--json', action='store_true', help='Save output file as json format')
    args = dt_parse.parse_args()

    soc_path = args.soc
    cfg = read_file(soc_path)

    conf = load_config_file()
    devkits = conf['devkits']

    for devkit in devkits:
        devkit = devkit.lower()
        if args.board in devkit:
            board = devkit

    if not board:
        print("Error: %s development kit is not supported\n" % args.board)
        return -1

    dts_filename = os.path.join(efinix_dir, board, 'linux', dts_filename)

    if args.outfile:
        output_filename = args.outfile

    # root
    root_node = dt_create_root_node()

    # bus
    bus_name = 'PERIPHERAL_BMB'
    bus_label = 'apbA'
    apb_node = dt_create_bus_node(cfg, bus_name, bus_label)

    # cpu
    cpu_node = dt_create_cpu_node(cfg)
    if cpu_node:
        root_node = dt_insert_child_node(root_node, cpu_node)

    # clock
    clk_node = dt_create_clock_node(cfg)
    if clk_node:
        root_node = dt_insert_child_node(root_node, clk_node)

    # plic
    plic_node = dt_create_plic_node(cfg)
    if plic_node:
        apb_node = dt_insert_child_node(apb_node, plic_node)

    for peripheral in ["UART", "SPI", "GPIO", "I2C"]:
        periph_node = dt_create_node(cfg, peripheral)
        if periph_node:
            apb_node = dt_insert_child_node(apb_node, periph_node)

    root_node = dt_insert_child_node(root_node, apb_node)

    out = dt_parser_nodes(root_node, root_node)
    save_file(output_filename, out)
    print("Info: device tree stored in %s" % output_filename)

    # create dts file
    dts_out = create_dts_file(cfg, apb_node)
    save_file(dts_filename, dts_out)
    print("Info: save dts of board %s in %s" % (board, dts_filename))

    # save in json format
    if args.json:
        with open(output_json, 'w') as f:
            json.dump(root_node, f, indent=4, sort_keys=False)

        print("Info: device tree json format stored in %s" % output_json)


if __name__ == "__main__":
    main()

