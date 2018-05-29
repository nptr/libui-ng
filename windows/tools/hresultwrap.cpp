// 21 may 2018
#include <vector>
#include <stdio.h>
#include <string.h>
#include "lib.hpp"

namespace {

class Function {
	ByteSlice name;
	ByteSlice callingConvention;
	std::vector<ByteSlice> params;
	ByteSlice returns;
	bool keepReturn;
	ByteSlice cond[2];
public:
	Function(ByteSlice line);

	ByteSlice Signature(void) const;
	ByteSlice Call(void) const;
	ByteSlice Body(void) const;
};

Function::Function(ByteSlice line)
{
	std::vector<ByteSlice> fields;
	size_t start;

	fields = ByteSliceFields(line);

	this->returns = fields[0];
	this->keepReturn = false;
	if (this->returns.Data()[0] == '*') {
		this->returns = this->returns.Slice(1, this->returns.Len());
		this->keepReturn = true;
	}

	start = 2;
	this->callingConvention = ByteSlice().AppendString(u8"WINAPI");
	this->name = fields[1];
	if (fields.size() % 2 == 1) {
		start = 3;
		this->callingConvention = fields[1];
		this->name = fields[2];
	}

	this->cond[1] = fields.back();
	fields.pop_back();
	this->cond[0] = fields.back();
	fields.pop_back();

	this->params = std::vector<ByteSlice>(fields.begin() + start, fields.end());
}

#define nfuncoutbuf 256

ByteSlice Function::Signature(void) const
{
	ByteSlice out;
	size_t i;

	out = ByteSlice(0, nfuncoutbuf);

	out = out.AppendString(u8"HRESULT ");
	out = out.Append(this->callingConvention);
	out = out.AppendString(u8" ");
	out = out.Append(this->name);

	out = out.AppendString(u8"(");
	for (i = 0; i < this->params.size(); i += 2) {
		out = out.Append(this->params[i]);
		out = out.AppendString(u8" ");
		out = out.Append(this->params[i + 1]);
		out = out.AppendString(u8", ");
	}
	if (this->keepReturn) {
		out = out.Append(this->returns);
		out = out.AppendString(u8" *ret");
	} else if (this->params.size() != 0)
		// remove the trailing comma and space
		out = out.Slice(0, out.Len() - 2);
	else
		out = out.AppendString(u8"void");
	out = out.AppendString(u8")");

	return out;
}

ByteSlice Function::Call(void) const
{
	ByteSlice out;
	size_t i;

	out = ByteSlice(0, nfuncoutbuf);
	out = out.Append(this->name);
	out = out.AppendString(u8"(");
	for (i = 0; i < this->params.size(); i += 2) {
		out = out.Append(this->params[i + 1]);
		out = out.AppendString(u8", ");
	}
	if (this->params.size() != 0)
		// remove the trailing comma and space
		out = out.Slice(0, out.Len() - 2);
	out = out.AppendString(u8")");
	return out;
}

#define nbodybuf 1024

ByteSlice Function::Body(void) const
{
	ByteSlice out;

	out = ByteSlice(0, nbodybuf);
	out = out.AppendString(u8"{\n");

	if (!this->keepReturn) {
		out = out.AppendString(u8"\t");
		out = out.Append(this->returns);
		out = out.AppendString(u8" ret;\n");
	}
	out = out.AppendString(u8"\tDWORD lasterr;\n");
	out = out.AppendString(u8"\n");

	if (this->keepReturn) {
		out = out.AppendString(u8"\tif (ret == NULL)\n");
		out = out.AppendString(u8"\t\treturn E_POINTER;\n");
	}

	out = out.AppendString(u8"\tSetLastError(0);\n");
	out = out.AppendString(u8"\t");
	if (this->keepReturn)
		out = out.AppendString(u8"*");
	out = out.AppendString(u8"ret = ");
	out = out.Append(this->Call());
	out = out.AppendString(u8";\n");
	out = out.AppendString(u8"\tlasterr = GetLastError();\n");

	out = out.AppendString(u8"\tif (");
	if (this->keepReturn)
		out = out.AppendString(u8"*");
	out = out.AppendString(u8"ret ");
	out = out.Append(this->cond[0]);
	out = out.AppendString(u8" ");
	out = out.Append(this->cond[1]);
	out = out.AppendString(u8")\n");
	out = out.AppendString(u8"\t\treturn lastErrorToHRESULT(lasterr, \"");
	out = out.Append(this->name);
	out = out.AppendString(u8"()\");\n");
	out = out.AppendString(u8"\treturn S_OK;\n");

	out = out.AppendString(u8"}");
	return out;
}

#define noutbuf 2048

ByteSlice generate(ByteSlice line)
{
	ByteSlice genout;
	Function *f;

	genout = ByteSlice(0, noutbuf);

	f = new Function(line);
	genout = genout.Append(f->Signature());
	genout = genout.AppendString(u8"\n");
	genout = genout.Append(f->Body());
	delete f;

	genout = genout.AppendString(u8"\n");
	genout = genout.AppendString(u8"\n");
	return genout;
}

ByteSlice process(ByteSlice line)
{
	if (line.Len() > 0 && line.Data()[0] == '@')
		return generate(line.Slice(1, line.Len()));
	return line.AppendString("\n");
}

}

int main(int argc, char *argv[])
{
	ReadCloser *fin = NULL;
	WriteCloser *fout = NULL;
	Scanner *s = NULL;
	ByteSlice b;
	int ret = 1;
	Error *err = NULL;

	if (argc != 3) {
		fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
		return 1;
	}

	err = OpenRead(argv[1], &fin);
	if (err != NULL) {
		fprintf(stderr, "error opening %s: %s\n", argv[1], err->String());
		goto done;
	}
	err = CreateWrite(argv[2], &fout);
	if (err != NULL) {
		fprintf(stderr, "error creating %s: %s\n", argv[2], err->String());
		goto done;
	}

	s = new Scanner(fin);
	while (s->Scan()) {
		b = process(s->Bytes());
		err = fout->Write(b);
		if (err != NULL) {
			fprintf(stderr, "error writing to %s: %s\n", argv[2], err->String());
			goto done;
		}
	}
	err = s->Err();
	if (err != NULL) {
		fprintf(stderr, "error reading from %s: %s\n", argv[1], err->String());
		err = NULL;		// we don't own err here
		goto done;
	}

	ret = 0;
done:
	if (s != NULL)
		delete s;
	if (fout != NULL)
		delete fout;
	if (fin != NULL)
		delete fin;
	if (err != NULL)
		delete err;
	return ret;
}
