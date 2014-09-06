linux_if
========

Linux network monitoring plugin with support for *bonding* and *vlans*.

Plugin will group all bonding slaves into one aggregated graph. It removes clutter
of having many single-metric graphs for individual interfaces. You still can click
trough to individual graphs.

In this example, `p1p1`, `p2p1` are physical interfaces of one bond0. Only p1p1 is active,
as we are using Active-Backup bonding mode. `Total` (black) is the throughput of parent
interface `bond0`.

![linux_if_bonding text](linux_if_bonding.png "linux_if bonding feature")

Similar aggregation is done also on higher level for vlans. All vlan sub-interfaces
are aggregated into one graph, based on their parent interface. Interfaces do not even
have to follow the same naming convention (_interface_._vlanid_). Interfaces `bond0.279`,
`vlan101`, `vlan102` are all vlan sub-interfaces of bond0.

![linux_if_vlans_text](linux_if_vlans.png "linux_if vlan feature")

