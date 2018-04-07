extensiondir = $(datadir)/@PACKAGE@/extensions
extension_DATA = *.ctr
bin_SCRIPTS = citron
install-data-local:
	rm -rf $(datadir)/@PACKAGE@/Library
	rm -rf $(datadir)/@PACKAGE@/mods
	rm -rf $(datadir)/@PACKAGE@/basemods
	cp -Lrf mods $(datadir)/@PACKAGE@/
	cp -Lrf Library $(datadir)/@PACKAGE@/
	cp -Lrf basemods $(datadir)/@PACKAGE@/
