// Console: Delinearizator
// Description: Delinearize input file
// Parameters: [input file] [output file]

/** Print help for delinearizator */
function delinearize_help() {
 print(tr("Usage:"));
 print("delinearize ["+tr("input file")+"] ["+tr("output file")+"]");
 print(" "+tr("Input file must exist"));
 print(" "+tr("Output file must not exist"));
 exit(1);
}

function delinearize_fail(err) {
 print(err);
 exit(2);
}

p=parameters();
if (p.length!=2) {
 delinearize_help("Delinearizator "+tr("is expecting two parameters")) 
}
inFile=p[0];
outFile=p[1];
if (!exists(inFile)) delinearize_fail(tr("Input file '%1' does not exist").arg(inFile));
if (exists(outFile)) delinearize_fail(tr("Output file '%1' already exist").arg(outFile));
if (delinearize(inFile,outFile)) {
 print(tr("Delinearized")+" :"+inFile+" -> "+outFile);
} else {
 delinearize_fail(error());
}
