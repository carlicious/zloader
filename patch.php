<?php

function VmaToOffset($lpVma) {
    return $lpVma - 0x41E00200;
}

function ToBinaryDword($dwDword) {
    return chr($dwDword) . chr($dwDword >> 8) . chr($dwDword >> 16) . chr($dwDword >> 24);
}

function ToBinaryQword($dwUpper, $dwLower) {
    return ToBinaryDword($dwLower) . ToBinaryDword($dwUpper);
}

function ReplaceBuf(&$abBuffer, $dwOffset, $abReplacement) {
    $abBuffer =
        substr($abBuffer, 0, $dwOffset) .
        $abReplacement .
        substr($abBuffer, $dwOffset + strlen($abReplacement));
}

if (count($argv) != 3) {
    printf("usage: %s <input file> <output file>\n", $argv[0]);
    exit(1);
}

$abZloader = @file_get_contents($argv[1]);
if (hash("sha256", $abZloader) !== "924281e12d5325ed71754b8748a89676aa6aec782e94f101d6caf21bc113138e") {
    printf("[-] unexpected input file\n");
    exit(1);
}

// ignore EngDebugFlag
ReplaceBuf($abZloader, VmaToOffset(0x41E004D0), "\x20\x00\x80\x52"); // MOV W0, #1
ReplaceBuf($abZloader, VmaToOffset(0x41E02A18), "\x1F\x20\x03\xD5"); // NOP
ReplaceBuf($abZloader, VmaToOffset(0x41E02B90), "\x1F\x20\x03\xD5"); // NOP
// ReplaceBuf($abZloader, VmaToOffset(0x41E050D8), "\x1F\x20\x03\xD5"); // NOP

// get_boot_flag
ReplaceBuf($abZloader, VmaToOffset(0x41E05028), file_get_contents("get_boot_flag.bin"));

// zld_swapBootImage
ReplaceBuf($abZloader, VmaToOffset(0x41E05314), file_get_contents("zld_swapBootImage.bin"));

// do_boot
ReplaceBuf($abZloader, VmaToOffset(0x41E051A8), file_get_contents("do_boot.bin"));
ReplaceBuf($abZloader, VmaToOffset(0x41E0261C), "\xe3\x0a\x00\x14"); // B do_boot_replace

file_put_contents($argv[2], $abZloader);

?>