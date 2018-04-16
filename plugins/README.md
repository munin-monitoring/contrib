# Contributed Munin Plugins

This plethora of plugins covering various topics was contributed by many different users of [munin](http://munin-monitoring.org).

See the [gallery](http://gallery.munin-monitoring.org/) for a browsable overview of these plugins.


## Purpose of this repository

This repository of contributed plugin strives to achieve the following goals:

* allow users to find interesting plugins
* allow contributors to publish their plugins
* simplify cooperative maintenance of plugins

Contributed plugins are maintained primarily by their authors.
You may file bug reports for plugin issue here in this repository (`munin-contrib`), but please do not forget to notify the author the plugin, as well.

Please note, that this repository is not supposed to be a dumping site for random plugins of low quality. The related infrastructure (e.g the [gallery](http://gallery.munin-monitoring.org/) or automated tests) require a certain level of quality. Please see below for details.


## Submit a new plugin

1. check if a similar plugin exists and if it can be extended/changed instead of adding a new plugin
    * please avoid code copies - they are a maintenance burden
2. add [documentation](http://guide.munin-monitoring.org/en/latest/develop/documenting.html#plugin-documentation) including configuration, author, license and [magic markers](http://guide.munin-monitoring.org/en/latest/architecture/syntax.html#magic-markers)
3. pick a suitable [category](http://guide.munin-monitoring.org/en/latest/reference/graph-category.html)
5. use style check tools for the language of the plugin (e.g. `shellcheck` for shell and `flake8` for Python)
6. pick a suitable [name and location](#Plugin_name_and_location)
7. bonus:
    * use the [multigraph approach](http://guide.munin-monitoring.org/en/latest/plugin/multigraphing.html#plugin-multigraphing) for non-trivial plugins
    * add [example graphs](http://munin-monitoring.org/wiki/PluginGallery#Rulesforplugincontributors) for the [gallery](http://gallery.munin-monitoring.org/)
    * support [dirtyconfig](http://guide.munin-monitoring.org/en/latest/plugin/protocol-dirtyconfig.html#plugin-protocol-dirtyconfig) if it is suitable
8. open a [pull request](https://github.com/munin-monitoring/contrib/pull/) with your new plugin or send it attached to an email to the [mailing list](https://lists.sourceforge.net/lists/listinfo/munin-users)

See the [plugin development documentation](http://guide.munin-monitoring.org/en/latest/develop/plugins/index.html) for more details.


## Modify an existing plugin

* *try* to keep the plugin backwards compatible (e.g. keep data fieldnames unchanged)
    * improvements of code quality and features can justify incompatible changes of existing plugins
* bonus:
    * improve the existing plugins according to the [wishlist for new plugins](#Submit_a_new_plugin)
    * upgrades from simple plugins to a [multigraph plugin](http://guide.munin-monitoring.org/en/latest/plugin/multigraphing.html#plugin-multigraphing) are welcome


## Plugin name and location

The following descriptions are *intentions* - they do not necessarily describe the current state for all plugins. Please open a [pull request](https://github.com/munin-monitoring/contrib/pull/) if you want to align the current structure along the goals outlined below:

* the top level directory should describe a related *software* or *vendor*
    * use *concepts* or *platforms* only if it is really necessary (e.g. *cpu*, *bsd*, *memory*)
* subdirectories are usually not required
