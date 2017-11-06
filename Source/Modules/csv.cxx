#include "swigmod.h"
#include <vector>

#define usage "CSV Options (csv parse tree of Plain Old Data)\n" \
"     -csvfilter    [name]    - Filter the definition of a class/struct by selecting the name\n" \
"     -csvdelimiter [delim]   - TODO delimiter of the CSV fields\n" \
"     -csvheader              - TODO Add csv header\n"

/* -----------------------------------------------------------------------------
 * \brief CSVMemberData contains member information
 * ----------------------------------------------------------------------------- */
struct CSVMemberData
{
public:
   CSVMemberData(String*givenName, String*givenType, String*givenKind, String*givenAccess, String*givenDecl) {
      mName = Copy(givenName);
      mType = Copy(givenType);
      mKind = Copy(givenKind);
      mAccess = Copy(givenAccess);
      mDecl = Copy(givenDecl);
   }
   ~CSVMemberData() {
      if(NULL == mName) {
         Delete(mName), mName = NULL;
      }
      if(NULL == mType) {
         Delete(mType), mType = NULL;
      }
      if(NULL == mKind) {
         Delete(mKind), mKind = NULL;
      }
      if(NULL == mAccess) {
         Delete(mAccess), mAccess = NULL;
      }
      if(NULL == mDecl) {
         Delete(mDecl), mDecl = NULL;
      }
   }
   void emit(File*output) {
      // member name | member type | member kind | member access | member declaration | Parameters (TODO)
      Printf(output, "|%s|%s|%s|%s|%s"
            , mName, mType, mKind, mAccess, mDecl);
   }
private:
   String *mName;
   String *mType;
   String *mKind;
   String *mAccess;
   String *mDecl;
};

/* -----------------------------------------------------------------------------
 * \brief CSVMetaData contains a container(class/struct) and the member
 *        variables. It also emits csv output of the metadata.
 * ----------------------------------------------------------------------------- */
struct CSVMetaData {
public:
   CSVMetaData() : containerName(NULL) {
   }
   ~CSVMetaData() {
      reset();
   }

   void reset() {
      if(NULL != containerName) { // release containerName
         Delete(containerName), containerName = NULL;
      }
      members.clear();
   }

   /* -----------------------------------------------------------------------------
    * \param   given    The name of the container(class/struct), can be set to NULL
    * ----------------------------------------------------------------------------- */
   void setContainerName(String *given) {
      if(NULL != containerName) {
         Delete(containerName), containerName = NULL;
      }

      if(NULL != given) {
         containerName = Copy(given);
      }
   }

   void addMember(String *givenName, String *givenType, String *givenKind, String *givenAccess, String *givenDecl) {
      CSVMemberData member(givenName, givenType, givenKind, givenAccess, givenDecl);
      members.push_back(member);
      // members.emplace_back(givenName, givenType, givenKind); // c++11 avoid memory copy
   }

   void emit(File*output) {
      unsigned int i = 0;
      for(i = 0; i < members.size(); i++) {
         // add a new row for each member
         if(containerName) { // first column is containerName
            Printf(output, "%s", containerName);
         }
         members.at(i).emit(output);
         Printf(output, "\n"); // end row
      }
      Printf(output, "\n"); // end container
   }

private:
   // Name of the container (class/struct)
   String *containerName;

   // list of members, each containing the name, type and kind tuple
   std::vector<struct CSVMemberData> members;
};

/* -----------------------------------------------------------------------------
 * \brief CSV is the parser/generator that generates the output.
 * ----------------------------------------------------------------------------- */
class CSVModule : public Language {
public:
   CSVModule()
   : filteredClassname(NULL)
      , s_csv(NULL)
      , verbose(false) {
   }

   ~CSVModule() {
      if(NULL  != filteredClassname) {
         Delete(filteredClassname), filteredClassname = NULL;
      }
      if(NULL  != s_csv) {
         Delete(s_csv), s_csv = NULL;
      }
   }

   // Entry point of the module
   virtual void main(int argc, char *argv[]) {
      for (int i = 1; i < argc; i++) {
         if(!argv[i]) { // skip NULL
            continue;
         }
         if (strcmp(argv[i],"-help") == 0) {
            // ------------------------------------------ help
            fputs(usage,stderr);
         } else if(strcmp(argv[i],"-csvfilter") == 0 && ((i+1) < argc) && argv[i+1]) {
            // ------------------------------------------ csvfilter
            if(verbose) {
               printf("Filtering class %s\n", argv[i+1]);
            }
            if(NULL  != filteredClassname) {
               Delete(filteredClassname);
               filteredClassname = NULL;
            }
            filteredClassname = NewString(argv[i+1]);
            Swig_mark_arg(i);
            Swig_mark_arg(i+1);
            i++; // skip the argument
         } else if(strcmp(argv[i], "-v") == 0) {
            // ------------------------------------------ verbose
            verbose = true;
         }
      }
      /* Add typemap definitions */
      SWIG_typemap_lang("csv");
      SWIG_library_directory("csv");
      SWIG_config_file("csv.swg");
      allow_overloading();
   }

   // Top of the parser stack
   virtual int top(Node *n) {
      String *outfile = Getattr(n, "outfile");

      log_verbose("Opening output file\n");
      File*f_csvfile = NewFile(outfile, "w", SWIG_output_files());
      if (!f_csvfile) {
         FileErrorDisplay(outfile);
         SWIG_exit(EXIT_FAILURE);
      } 

      /* Initialize I/O */
      // -------------- historical code start
      File*f_begin = NewString("");
      File*f_header = NewString("");
      File*f_runtime = NewString("");
      // -------------- historical code end

      // CSV IO
      File*s_csvbanner = NewString("");
      s_csv = NewString("");

      /* Register file targets with the SWIG file handler */
      // -------------- historical code start
      Swig_register_filebyname("begin", f_begin);
      Swig_register_filebyname("header", f_header);
      Swig_register_filebyname("runtime", f_runtime);
      // -------------- historical code end

      // CSV code insert
      Swig_register_filebyname("csvbanner", s_csvbanner);
      Swig_register_filebyname("csv", s_csv);

      /* Emit code for children */
      Language::top(n);

      /* Close all of the files */
      Dump(s_csvbanner, f_csvfile); // banner
      Dump(s_csv, f_csvfile);       // content

      // cleanup
      // -------------- historical code start
      Delete(f_runtime);
      f_runtime = NULL;
      Delete(f_header);
      f_header = NULL;
      Delete(f_begin);
      // -------------- historical code end
      f_begin = NULL;
      Delete(f_csvfile);
      f_csvfile = NULL;
      Delete(s_csv);
      s_csv = NULL;
      Delete(s_csvbanner);
      s_csvbanner = NULL;

      log_verbose("Code generation complete\n");
      return SWIG_OK;
   }

   // Parse and push the current class in the metaData
   virtual int classDeclaration(Node *n)
   {
      String *name = Getattr(n, "sym:name");
      // filter by classname
      if((NULL == filteredClassname) || (Strcmp(name, filteredClassname) == 0))
      {
         metaData.reset(); // empty the metadata
         // push it in the metaData
         metaData.setContainerName(name);
         emit_children(n);
         log_verbose("Emitting metadata\n");
         metaData.emit(s_csv);
         metaData.reset(); // empty the metadata
      }
      return SWIG_OK;
   }

   // Parse definition of the variable
   virtual int cDeclaration(Node *n)
   {
      String *name = Getattr(n, "sym:name");
      String *memberType = Getattr(n, "type");
      String *memberKind = Getattr(n, "kind");
      String *memberAccess = Getattr(n, "access");
      String *memberDecl = Getattr(n, "decl");
      // push it in the metaData
      metaData.addMember(name, memberType, memberKind, memberAccess, memberDecl);
      return SWIG_OK;
   }

   void log_verbose(const char*str)
   {
      if(verbose)
      {
         puts(str);
      }
   }

private:
   String *filteredClassname;
   File *s_csv; // output buffer
   bool verbose;

   // capture symbols
   struct CSVMetaData metaData;
};

/* -----------------------------------------------------------------------------
 * swig_csv()    - Instantiate module
 * ----------------------------------------------------------------------------- */

static Language *new_swig_csv() {
  return new CSVModule();
}
extern "C" Language *swig_csv(void) {
  return new_swig_csv();
}


