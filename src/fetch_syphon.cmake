
# Download Syphon framework
file(DOWNLOAD
  https://github.com/Syphon/Syphon-Framework/releases/download/5/Syphon.SDK.5.zip
  Syphon.zip
  EXPECTED_HASH SHA256=8a8993da2d39f84b7fbd9ad0071a8f300e947635a52c6c5a521c88df5ccb882f
  TLS_VERIFY true
)

file(ARCHIVE_EXTRACT
    INPUT Syphon.zip
    DESTINATION .
    PATTERNS "Syphon SDK 5/Syphon.framework/"
)