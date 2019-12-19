FROM photon:3.0
COPY . /xqviewer
RUN rm /xqviewer/*.sh -f
WORKDIR /xqviewer
ENTRYPOINT ["/xqviewer/xqviewer"]
