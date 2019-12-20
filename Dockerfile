FROM photon:3.0
COPY . /xqviewer
RUN rm /xqviewer/*.sh /xqviewer/Dockerfile /xqviewer/*.c /xqviewer/.git* -rf
WORKDIR /xqviewer
ENTRYPOINT ["/xqviewer/xqviewer"]
