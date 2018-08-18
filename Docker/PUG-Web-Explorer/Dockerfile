########################################
# Derived from ubuntu distribution
########################################
FROM iitdbgroup/gprom-ubuntu-all-backends:pug
MAINTAINER Boris Glavic <bglavic@iit.edu>
########################################
# Install python
########################################
RUN apt-get -y update && apt-get install -y \
	python \
	python-pip \
	&& rm -rf /var/lib/apt/lists/*
########################################
# Copy connection settings
########################################
RUN mv /gprom/src/clients/pythonweb /web
COPY connsettings.py /web/provgraph_web/
COPY provgraph_web.db /web/provgraph_web/
########################################
# install python packages
########################################
RUN pip install -r /web/provgraph_web/requirements.txt
########################################
# Expose webserver port
########################################
EXPOSE 5000
########################################
# Per default start the gprom command line interface
########################################
ENTRYPOINT ["/web/manage.py"]
########################################
# Per start the webserver
########################################
CMD ["startserver"]
