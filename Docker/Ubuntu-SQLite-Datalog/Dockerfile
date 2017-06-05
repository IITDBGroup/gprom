########################################
# Derived from ubuntu distribution
########################################
FROM iitdbgroup/gprom-ubuntu-example-sqlite:latest
MAINTAINER Boris Glavic <bglavic@iit.edu>
########################################
# Per default start the gprom command line interface
########################################
ENTRYPOINT ["/usr/local/bin/gprom"]
########################################
# Per default connect to the example SQLite database
########################################
CMD ["-frontend", "dl", "-backend", "sqlite", "-db", "/gprom/src/examples/test.db"]
