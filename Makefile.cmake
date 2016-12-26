netfileserver: netfileserver.o
	$(CC) -o $@ @^
clean:
	rm netfileserver