dist.tar.gz: bootstrap.squashfs | ro rw work
	tar -czf $@ $+ $|

bootstrap.squashfs: | rw
	./build-bootstrap

ro rw work:
	mkdir $@
