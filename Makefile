dist.tar.gz: bootstrap.squashfs activate deactivate meteor-wrapper install uninstall | ro rw work
	tar -czf $@ $+ $|

bootstrap.squashfs: | rw
	./build-bootstrap

ro rw work:
	mkdir $@
