<template>
  <div class="rImage">
      <img :src="('/' + path)">
      <i id="trash" class="fa fa-trash fa-lg" aria-hidden="false" style="color:red;" @click="delete_it(path)"></i>
  </div>
</template>

<script>
export default {
  name: 'rImage',
  props: {
    path: String
  },
  methods: {
    test_it(path){
      this.$parent.refresh();
    },
    delete_it(path) {
      if(confirm("Möchten Sie das Bild wirklich Löschen")){
        console.log("delete " + path)
        let theURL = "http://127.0.0.1:5000/undo?key=image&img_path='"+path+"'"
        let currentObj = this;
        this.axios.get(theURL).then(function (response) {
          console.log("deleted")
          currentObj.$parent.refresh();

        }).catch(function (error) {
          currentObj.output = error;
        });
      }

    }
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.rImage {
  width: 120px;
  height: 120px;
  display: inline;
  float: left;
  position: relative;
  margin: 8px;
}
img {
  max-width: 100%;
  max-height: 100%;
}
#trash {
  position: absolute;
  top: 10%;
  left: 10%;
  transform: translate(-50%, -50%);
}

</style>
