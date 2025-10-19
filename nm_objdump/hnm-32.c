#include "hnm.h"

/**
 * get_symbol_type32 - determines the type of a 32-bit ELF symbol
 * @symbol: ELF symbol structure
 * @section_headers: pointer to ELF section headers
 * Return: character representing symbol type
 */
char get_symbol_type32(Elf32_Sym symbol, Elf32_Shdr *section_headers)
{
	char type = '?';
	Elf32_Shdr sec;

	if (ELF32_ST_BIND(symbol.st_info) == STB_WEAK)
	{
		if (symbol.st_shndx == SHN_UNDEF)
			return ('w');
		if (ELF32_ST_TYPE(symbol.st_info) == STT_OBJECT)
			return ('V');
		return ('W');
	}
	if (symbol.st_shndx == SHN_UNDEF)
		return ('U');
	if (symbol.st_shndx == SHN_ABS)
		return ('A');
	if (symbol.st_shndx == SHN_COMMON)
		return ('C');
	if (symbol.st_shndx < SHN_LORESERVE)
	{
		sec = section_headers[symbol.st_shndx];
		if (ELF32_ST_BIND(symbol.st_info) == STB_GNU_UNIQUE)
			type = 'u';
		else if (sec.sh_type == SHT_NOBITS &&
			 sec.sh_flags == (SHF_ALLOC | SHF_WRITE))
			type = 'B';
		else if (sec.sh_type == SHT_PROGBITS)
		{
			if (sec.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
				type = 'T';
			else if (sec.sh_flags == SHF_ALLOC)
				type = 'R';
			else if (sec.sh_flags == (SHF_ALLOC | SHF_WRITE))
				type = 'D';
		}
		else if (sec.sh_type == SHT_DYNAMIC)
			type = 'D';
		else
			type = 't';
	}
	return (type);
}

/**
 * print_symbol_table32 - prints the 32-bit ELF symbol table
 * @shdr: pointer to symbol table section header
 * @symtab: pointer to ELF symbol table
 * @strtab: pointer to string table
 * @sh_table: pointer to section headers
 * Return: void
 */
void print_symbol_table32(Elf32_Shdr *shdr, Elf32_Sym *symtab,
			  char *strtab, Elf32_Shdr *sh_table)
{
	int i, sym_count;
	Elf32_Sym sym;
	char type, *name;

	sym_count = shdr->sh_size / sizeof(Elf32_Sym);
	for (i = 0; i < sym_count; i++)
	{
		sym = symtab[i];
		if (sym.st_name == 0 || ELF32_ST_TYPE(sym.st_info) == STT_FILE)
			continue;
		name = strtab + sym.st_name;
		type = get_symbol_type32(sym, sh_table);
		if (ELF32_ST_BIND(sym.st_info) == STB_LOCAL)
			type = tolower(type);
		if (type != 'U' && type != 'w')
			printf("%08x %c %s\n", sym.st_value, type, name);
		else
			printf("         %c %s\n", type, name);
	}
}

/**
 * read_section_headers32 - reads section headers from ELF file
 * @file: file pointer
 * @hdr: ELF header
 * Return: pointer to allocated section headers or NULL
 */
Elf32_Shdr *read_section_headers32(FILE *file, Elf32_Ehdr hdr)
{
	Elf32_Shdr *shdrs;

	shdrs = malloc(hdr.e_shentsize * hdr.e_shnum);
	if (!shdrs)
		return (NULL);
	fseek(file, hdr.e_shoff, SEEK_SET);
	fread(shdrs, hdr.e_shentsize, hdr.e_shnum, file);
	return (shdrs);
}

/**
 * process_elf_file32 - processes and prints symbols of a 32-bit ELF file
 * @file_path: ELF file path
 * Return: void
 */
void process_elf_file32(char *file_path)
{
	FILE *file;
	Elf32_Ehdr hdr;
	Elf32_Shdr *shdrs, sym_hdr, str_hdr;
	Elf32_Sym *symtab;
	char *strtab;
	int i, sym_index = -1;

	file = fopen(file_path, "rb");
	if (!file)
	{
		fprintf(stderr, "./hnm: %s: failed to open file\n", file_path);
		return;
	}
	fread(&hdr, sizeof(hdr), 1, file);
	if (hdr.e_ident[EI_CLASS] != ELFCLASS32)
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file format\n", file_path);
		fclose(file);
		return;
	}
	shdrs = read_section_headers32(file, hdr);
	if (!shdrs)
	{
		fprintf(stderr, "./hnm: %s: memory allocation error\n", file_path);
		fclose(file);
		return;
	}
	for (i = 0; i < hdr.e_shnum; i++)
		if (shdrs[i].sh_type == SHT_SYMTAB)
			sym_index = i;
	if (sym_index == -1)
	{
		fprintf(stderr, "./hnm: %s: no symbols\n", file_path);
		free(shdrs);
		fclose(file);
		return;
	}
	sym_hdr = shdrs[sym_index];
	symtab = malloc(sym_hdr.sh_size);
	fseek(file, sym_hdr.sh_offset, SEEK_SET);
	fread(symtab, sym_hdr.sh_size, 1, file);
	str_hdr = shdrs[sym_hdr.sh_link];
	strtab = malloc(str_hdr.sh_size);
	fseek(file, str_hdr.sh_offset, SEEK_SET);
	fread(strtab, str_hdr.sh_size, 1, file);
	print_symbol_table32(&sym_hdr, symtab, strtab, shdrs);
	free(shdrs);
	free(symtab);
	free(strtab);
	fclose(file);
}

