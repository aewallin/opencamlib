Module['preInit'] = []
Module['preInit'].push(function () {
    FS.mkdir('/mnt');
    FS.mount(NODEFS, { root: './stl/' }, '/mnt');
})
