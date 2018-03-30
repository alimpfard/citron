extensiondir = $(datadir)/@PACKAGE@/extensions
extension_DATA = *.ctr
bin_SCRIPTS = citron
install-data-local:
	rm -rf $(datadir)/@PACKAGE@/Library
	rm -rf $(datadir)/@PACKAGE@/mods
	cp -Lrf mods $(datadir)/@PACKAGE@/
	cp -Lrf Library $(datadir)/@PACKAGE@/
