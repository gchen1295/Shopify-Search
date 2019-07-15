{
  "targets": [
    {
      "target_name": "searcher",
      "sources": [ 
        "./cppsrc/searchwrapper.cc",
        "./cppsrc/product.cpp",
        "./cppsrc/searcher.cpp",
        "./cppsrc/verify.cpp"
        ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "D:\\Desktop\\vcpkg-master\\installed\\x64-windows\\include**"
      ],
      'libraries': [
        "D:\\Desktop\\vcpkg-master\\installed\\x64-windows\\lib\\**"
      ],
    }
  ]
}


