/* Delinearizator */

/** Print help for delinearizator */
function delinearize_help() {
 print("Usage:");
 print("Delinearizator [input file] [output file]");
 print(" Input file must exist");
 print(" Output file must not exist");
 exitApp(1);
}

function delinearize_fail(err) {
 print(err);
 exitApp(2);
}

p=parameters();
if (p.length!=2) {
 delinearize_help("Delinearizator expect two parameters") 
}
inFile=p[0];
outFile=p[1];
if (!exists(inFile)) delinearize_fail("Input file '"+inFile+"' does not exist");
if (exists(outFile)) delinearize_fail("Output file '"+outFile+"' already exist");
if (delinearize(inFile,outFile)) {
 print("Delinearized:");
 print(inFile+" -> "+outFile);
} else {
 delinearize_fail(error());
}