#include <xml_node.h>

#ifdef DEBUG
static void debug_node_dealloc(xmlNodePtr x)
{
  NOKOGIRI_DEBUG_START(x)
  NOKOGIRI_DEBUG_END(x)
}
#else
#  define debug_node_dealloc 0
#endif

/*
 * call-seq:
 *  pointer_id
 *
 * Get the internal pointer number
 */
static VALUE pointer_id(VALUE self)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);

  return INT2NUM((int)(node));
}

/*
 * call-seq:
 *  encode_special_chars(string)
 *
 * Encode any special characters in +string+
 */
static VALUE encode_special_chars(VALUE self, VALUE string)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);
  xmlChar * encoded = xmlEncodeSpecialChars(
      node->doc,
      (const xmlChar *)StringValuePtr(string)
  );

  VALUE encoded_str = rb_str_new2((const char *)encoded);
  xmlFree(encoded);

  return encoded_str;
}

/*
 * call-seq:
 *  internal_subset
 *
 * Get the internal subset
 */
static VALUE internal_subset(VALUE self)
{
  xmlNodePtr node;
  xmlDocPtr doc;
  Data_Get_Struct(self, xmlNode, node);

  if(!node->doc) return Qnil;

  doc = node->doc;
  xmlDtdPtr dtd = xmlGetIntSubset(doc);

  if(!dtd) return Qnil;

  return Nokogiri_wrap_xml_node((xmlNodePtr)dtd);
}

/*
 * call-seq:
 *  dup
 *
 * Copy this node.  An optional depth may be passed in, but it defaults
 * to a deep copy.  0 is a shallow copy, 1 is a deep copy.
 */
static VALUE duplicate_node(int argc, VALUE *argv, VALUE self)
{
  VALUE level;

  if(rb_scan_args(argc, argv, "01", &level) == 0)
    level = INT2NUM(1);

  xmlNodePtr node, dup;
  Data_Get_Struct(self, xmlNode, node);

  dup = xmlDocCopyNode(node, node->doc, NUM2INT(level));
  if(dup == NULL) return Qnil;

  return Nokogiri_wrap_xml_node(dup);
}

/*
 * call-seq:
 *  unlink
 *
 * Unlink this node from its current context.
 */
static VALUE unlink_node(VALUE self)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);
  xmlUnlinkNode(node);
  return self;
}

/*
 * call-seq:
 *  blank?
 *
 * Is this node blank?
 */
static VALUE blank_eh(VALUE self)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);
  if(1 == xmlIsBlankNode(node))
    return Qtrue;
  return Qfalse;
}

/*
 * call-seq:
 *  next_sibling
 *
 * Returns the next sibling node
 */
static VALUE next_sibling(VALUE self)
{
  xmlNodePtr node, sibling;
  Data_Get_Struct(self, xmlNode, node);

  sibling = node->next;
  if(!sibling) return Qnil;

  return Nokogiri_wrap_xml_node(sibling) ;
}

/*
 * call-seq:
 *  previous_sibling
 *
 * Returns the previous sibling node
 */
static VALUE previous_sibling(VALUE self)
{
  xmlNodePtr node, sibling;
  Data_Get_Struct(self, xmlNode, node);

  sibling = node->prev;
  if(!sibling) return Qnil;

  return Nokogiri_wrap_xml_node(sibling);
}

/* :nodoc: */
static VALUE replace(VALUE self, VALUE _new_node)
{
  xmlNodePtr node, new_node;
  Data_Get_Struct(self, xmlNode, node);
  Data_Get_Struct(_new_node, xmlNode, new_node);

  xmlReplaceNode(node, new_node);
  return self ;
}


/*
 * call-seq:
 *  child
 *
 * Returns the child node
 */
static VALUE child(VALUE self)
{
  xmlNodePtr node, child;
  Data_Get_Struct(self, xmlNode, node);

  child = node->children;
  if(!child) return Qnil;

  return Nokogiri_wrap_xml_node(child);
}

/*
 * call-seq:
 *  key?(attribute)
 *
 * Returns true if +attribute+ is set
 */
static VALUE key_eh(VALUE self, VALUE attribute)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);
  if(xmlHasProp(node, (xmlChar *)StringValuePtr(attribute)))
    return Qtrue;
  return Qfalse;
}

/*
 * call-seq:
 *  []=(property, value)
 *
 * Set the +property+ to +value+
 */
static VALUE set(VALUE self, VALUE property, VALUE value)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);

  xmlChar *buffer = xmlEncodeEntitiesReentrant(node->doc,
      (xmlChar *)StringValuePtr(value));

  xmlSetProp(node, (xmlChar *)StringValuePtr(property), buffer);
  xmlFree(buffer);

  return value;
}

/*
 * call-seq:
 *   get(attribute)
 *
 * Get the value for +attribute+
 */
static VALUE get(VALUE self, VALUE attribute)
{
  xmlNodePtr node;
  xmlChar* propstr ;
  VALUE rval ;
  Data_Get_Struct(self, xmlNode, node);

  if(attribute == Qnil) return Qnil;

  propstr = xmlGetProp(node, (xmlChar *)StringValuePtr(attribute));

  if(NULL == propstr) return Qnil;

  rval = rb_str_new2((char *)propstr) ;
  xmlFree(propstr);
  return rval ;
}

/*
 * call-seq:
 *   attribute(name)
 *
 * Get the attribute node with +name+
 */
static VALUE attr(VALUE self, VALUE name)
{
  xmlNodePtr node;
  xmlAttrPtr prop;
  Data_Get_Struct(self, xmlNode, node);
  prop = xmlHasProp(node, (xmlChar *)StringValuePtr(name));

  if(! prop) return Qnil;
  return Nokogiri_wrap_xml_node((xmlNodePtr)prop);
}

/*
 *  call-seq:
 *    attribute_nodes()
 *
 *  returns a list containing the Node attributes.
 */
static VALUE attribute_nodes(VALUE self)
{
    /* this code in the mode of xmlHasProp() */
    xmlNodePtr node ;
    VALUE attr ;

    attr = rb_ary_new() ;
    Data_Get_Struct(self, xmlNode, node);

    Nokogiri_xml_node_properties(node, attr);

    return attr ;
}


/*
 *  call-seq:
 *    namespace()
 *
 *  returns the namespace prefix for the node, if one exists.
 */
static VALUE namespace(VALUE self)
{
  xmlNodePtr node ;
  Data_Get_Struct(self, xmlNode, node);
  if (node->ns && node->ns->prefix)
    return rb_str_new2((const char *)node->ns->prefix) ;
  return Qnil ;
}

/*
 *  call-seq:
 *    namespaces()
 *
 *  returns a hash containing the node's namespaces.
 */
static VALUE namespaces(VALUE self)
{
    /* this code in the mode of xmlHasProp() */
    xmlNodePtr node ;
    VALUE attr ;

    attr = rb_hash_new() ;
    Data_Get_Struct(self, xmlNode, node);

    Nokogiri_xml_node_namespaces(node, attr);

    return attr ;
}

/*
 * call-seq:
 *  type
 *
 * Get the type for this node
 */
static VALUE type(VALUE self)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);
  return INT2NUM((int)node->type);
}

/*
 * call-seq:
 *  content=
 *
 * Set the content for this Node
 */
static VALUE set_content(VALUE self, VALUE content)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);
  xmlNodeSetContent(node, (xmlChar *)StringValuePtr(content));
  return content;
}

/*
 * call-seq:
 *  content
 *
 * Returns the content for this Node
 */
static VALUE get_content(VALUE self)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);

  xmlChar * content = xmlNodeGetContent(node);
  if(content) {
    VALUE rval = rb_str_new2((char *)content);
    xmlFree(content);
    return rval;
  }
  return Qnil;
}

/*
 * call-seq:
 *  add_child(node)
 *
 * Add +node+ as a child of this node. Returns the new child node.
 */
static VALUE add_child(VALUE self, VALUE child)
{
  xmlNodePtr node, parent, new_child;
  Data_Get_Struct(child, xmlNode, node);
  Data_Get_Struct(self, xmlNode, parent);

  xmlUnlinkNode(node) ;

  if(!(new_child = xmlAddChild(parent, node)))
    rb_raise(rb_eRuntimeError, "Could not add new child");

  // the child was a text node that was coalesced. we need to have the object
  // point at SOMETHING, or we'll totally bomb out.
  if (new_child != node)
    DATA_PTR(child) = new_child ;

  return Nokogiri_wrap_xml_node(new_child);
}

/*
 * call-seq:
 *  parent
 *
 * Get the parent Node for this Node
 */
static VALUE get_parent(VALUE self)
{
  xmlNodePtr node, parent;
  Data_Get_Struct(self, xmlNode, node);

  parent = node->parent;
  if(!parent) return Qnil;

  return Nokogiri_wrap_xml_node(parent) ;
}

/*
 * call-seq:
 *  name=(new_name)
 *
 * Set the name for this Node
 */
static VALUE set_name(VALUE self, VALUE new_name)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);
  xmlNodeSetName(node, (xmlChar*)StringValuePtr(new_name));
  return new_name;
}

/*
 * call-seq:
 *  name
 *
 * Returns the name for this Node
 */
static VALUE get_name(VALUE self)
{
  xmlNodePtr node;
  Data_Get_Struct(self, xmlNode, node);
  if(node->name) return rb_str_new2((const char *)node->name);
  return Qnil;
}

/*
 * call-seq:
 *  path
 *
 * Returns the path associated with this Node
 */
static VALUE path(VALUE self)
{
  xmlNodePtr node;
  xmlChar *path ;
  VALUE rval ;
  Data_Get_Struct(self, xmlNode, node);
  
  path = xmlGetNodePath(node);
  rval = rb_str_new2((char *)path);
  xmlFree(path);
  return rval ;
}

/*
 *  call-seq:
 *    add_next_sibling(node)
 *
 *  Insert +node+ after this node (as a sibling).
 */
static VALUE add_next_sibling(VALUE self, VALUE rb_node)
{
  xmlNodePtr node, _new_sibling, new_sibling;
  Data_Get_Struct(self, xmlNode, node);
  Data_Get_Struct(rb_node, xmlNode, _new_sibling);

  if(!(new_sibling = xmlAddNextSibling(node, _new_sibling)))
    rb_raise(rb_eRuntimeError, "Could not add next sibling");

  // the sibling was a text node that was coalesced. we need to have the object
  // point at SOMETHING, or we'll totally bomb out.
  if(new_sibling != _new_sibling) DATA_PTR(rb_node) = new_sibling;

  rb_funcall(rb_node, rb_intern("decorate!"), 0);

  return rb_node;
}

/*
 * call-seq:
 *  add_previous_sibling(node)
 *
 * Insert +node+ before this node (as a sibling).
 */
static VALUE add_previous_sibling(VALUE self, VALUE rb_node)
{
  xmlNodePtr node, sibling, new_sibling;
  Check_Type(rb_node, T_DATA);

  Data_Get_Struct(self, xmlNode, node);
  Data_Get_Struct(rb_node, xmlNode, sibling);

  if(!(new_sibling = xmlAddPrevSibling(node, sibling)))
    rb_raise(rb_eRuntimeError, "Could not add previous sibling");

  // the sibling was a text node that was coalesced. we need to have the object
  // point at SOMETHING, or we'll totally bomb out.
  if(sibling != new_sibling) DATA_PTR(rb_node) = new_sibling;

  rb_funcall(rb_node, rb_intern("decorate!"), 0);

  return rb_node;
}

/*
 * call-seq:
 *  to_html
 *
 * Returns this node as HTML
 */
static VALUE to_html(VALUE self)
{
  xmlBufferPtr buf ;
  xmlNodePtr node ;
  Data_Get_Struct(self, xmlNode, node);

  VALUE html;

  if(node->doc->type == XML_DOCUMENT_NODE)
    return rb_funcall(self, rb_intern("to_xml"), 0);

  buf = xmlBufferCreate() ;
  htmlNodeDump(buf, node->doc, node);
  html = rb_str_new2((char*)buf->content);
  xmlBufferFree(buf);
  return html ;
}

/*
 * call-seq:
 *  to_xml
 *
 * Returns this node as XML
 */
static VALUE to_xml(int argc, VALUE *argv, VALUE self)
{
  xmlBufferPtr buf ;
  xmlNodePtr node ;
  VALUE xml, level;

  if(rb_scan_args(argc, argv, "01", &level) == 0)
    level = INT2NUM(1);

  Check_Type(level, T_FIXNUM);

  Data_Get_Struct(self, xmlNode, node);

  buf = xmlBufferCreate() ;
  xmlNodeDump(buf, node->doc, node, 2, NUM2INT(level));
  xml = rb_str_new2((char*)buf->content);
  xmlBufferFree(buf);
  return xml ;
}


/*
 * call-seq:
 *   new(name)
 *
 * Create a new node with +name+
 */
static VALUE new(VALUE klass, VALUE name, VALUE document)
{
  xmlDocPtr doc;

  Data_Get_Struct(document, xmlDoc, doc);

  xmlNodePtr node = xmlNewNode(NULL, (xmlChar *)StringValuePtr(name));
  node->doc = doc;

  VALUE rb_node = Nokogiri_wrap_xml_node(node);

  if(rb_block_given_p()) rb_yield(rb_node);

  return rb_node;
}

VALUE Nokogiri_wrap_xml_node(xmlNodePtr node)
{
  assert(node);
  assert(node->doc);
  assert(node->doc->_private);

  VALUE index = INT2NUM((int)node);
  VALUE document = (VALUE)node->doc->_private;

  VALUE node_cache = rb_funcall(document, rb_intern("node_cache"), 0);
  VALUE rb_node = rb_hash_aref(node_cache, index);

  if(rb_node != Qnil) return rb_node;

  switch(node->type)
  {
    VALUE klass;

    case XML_TEXT_NODE:
      klass = rb_const_get(mNokogiriXml, rb_intern("Text"));
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    case XML_ENTITY_REF_NODE:
      klass = cNokogiriXmlEntityReference;
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    case XML_COMMENT_NODE:
      klass = cNokogiriXmlComment;
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    case XML_DOCUMENT_FRAG_NODE:
      klass = cNokogiriXmlDocumentFragment;
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    case XML_PI_NODE:
      klass = cNokogiriXmlProcessingInstruction;
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    case XML_ELEMENT_NODE:
      klass = rb_const_get(mNokogiriXml, rb_intern("Element"));
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    case XML_ATTRIBUTE_NODE:
      klass = cNokogiriXmlAttr;
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    case XML_ENTITY_DECL:
      klass = rb_const_get(mNokogiriXml, rb_intern("EntityDeclaration"));
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    case XML_CDATA_SECTION_NODE:
      klass = cNokogiriXmlCData;
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    case XML_DTD_NODE:
      klass = rb_const_get(mNokogiriXml, rb_intern("DTD"));
      rb_node = Data_Wrap_Struct(klass, 0, debug_node_dealloc, node) ;
      break;
    default:
      rb_node = Data_Wrap_Struct(cNokogiriXmlNode, 0, debug_node_dealloc, node) ;
  }

  rb_hash_aset(node_cache, index, rb_node);
  rb_iv_set(rb_node, "@document", document);
  rb_funcall(rb_node, rb_intern("decorate!"), 0);
  return rb_node ;
}


void Nokogiri_xml_node_properties(xmlNodePtr node, VALUE attr_list)
{
  xmlAttrPtr prop;
  prop = node->properties ;
  while (prop != NULL) {
    rb_ary_push(attr_list, Nokogiri_wrap_xml_node((xmlNodePtr)prop));
    prop = prop->next ;
  }
}


#define XMLNS_PREFIX "xmlns"
#define XMLNS_PREFIX_LEN 6 /* including either colon or \0 */
#define XMLNS_BUFFER_LEN 128
void Nokogiri_xml_node_namespaces(xmlNodePtr node, VALUE attr_hash)
{
  xmlNsPtr ns;
  static char buffer[XMLNS_BUFFER_LEN] ;
  char *key ;
  size_t keylen ;

  if (node->type != XML_ELEMENT_NODE) return ;

  ns = node->nsDef;
  while (ns != NULL) {

    keylen = XMLNS_PREFIX_LEN + (ns->prefix ? (strlen((const char*)ns->prefix) + 1) : 0) ;
    if (keylen > XMLNS_BUFFER_LEN) {
      key = (char*)malloc(keylen) ;
    } else {
      key = buffer ;
    }

    if (ns->prefix) {
      sprintf(key, "%s:%s", XMLNS_PREFIX, ns->prefix);
    } else {
      sprintf(key, "%s", XMLNS_PREFIX);
    }

    rb_hash_aset(attr_hash, rb_str_new2(key), rb_str_new2((const char*)ns->href)) ;
    if (key != buffer) {
      free(key);
    }
    ns = ns->next ;
  }
}


VALUE cNokogiriXmlNode ;
void init_xml_node()
{
  /*
  VALUE nokogiri = rb_define_module("Nokogiri");
  VALUE xml = rb_define_module_under(nokogiri, "XML");
  VALUE klass = rb_define_class_under(xml, "Node", rb_cObject);
  */

  VALUE klass = cNokogiriXmlNode = rb_const_get(mNokogiriXml, rb_intern("Node"));

  rb_define_singleton_method(klass, "new", new, 2);

  rb_define_method(klass, "node_name", get_name, 0);
  rb_define_method(klass, "node_name=", set_name, 1);
  rb_define_method(klass, "add_child", add_child, 1);
  rb_define_method(klass, "parent", get_parent, 0);
  rb_define_method(klass, "child", child, 0);
  rb_define_method(klass, "next_sibling", next_sibling, 0);
  rb_define_method(klass, "previous_sibling", previous_sibling, 0);
  rb_define_method(klass, "type", type, 0);
  rb_define_method(klass, "content", get_content, 0);
  rb_define_method(klass, "path", path, 0);
  rb_define_method(klass, "key?", key_eh, 1);
  rb_define_method(klass, "blank?", blank_eh, 0);
  rb_define_method(klass, "[]=", set, 2);
  rb_define_method(klass, "attribute_nodes", attribute_nodes, 0);
  rb_define_method(klass, "attribute", attr, 1);
  rb_define_method(klass, "namespace", namespace, 0);
  rb_define_method(klass, "namespaces", namespaces, 0);
  rb_define_method(klass, "add_previous_sibling", add_previous_sibling, 1);
  rb_define_method(klass, "add_next_sibling", add_next_sibling, 1);
  rb_define_method(klass, "encode_special_chars", encode_special_chars, 1);
  rb_define_method(klass, "to_xml", to_xml, -1);
  rb_define_method(klass, "to_html", to_html, 0);
  rb_define_method(klass, "dup", duplicate_node, -1);
  rb_define_method(klass, "unlink", unlink_node, 0);
  rb_define_method(klass, "internal_subset", internal_subset, 0);
  rb_define_method(klass, "pointer_id", pointer_id, 0);

  rb_define_private_method(klass, "replace_with_node", replace, 1);
  rb_define_private_method(klass, "native_content=", set_content, 1);
  rb_define_private_method(klass, "get", get, 1);
}
